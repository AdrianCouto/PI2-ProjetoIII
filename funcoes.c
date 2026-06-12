#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include "minimips.h"
#include "hazards.h"

FILE *arquivo, *arquivoMemDados;

int contaLinhas(char *arq){
    arquivo = fopen(arq, "r");
    char ch;
    int count=0;

    if(arquivo==NULL){
        return 0;
    }

    while((ch=fgetc(arquivo))!=EOF){
        if(ch=='\n'){
            count++;
        }
    }

    fclose(arquivo);
    return count;
}

void lerMem(char *arq, instrucao **memoria, int linhas){
    *memoria = calloc(256, sizeof(instrucao));
    if(*memoria == NULL){
        return;
    }
    arquivo = fopen(arq, "r");
    int i=0;
    char mem[17];

    if(arquivo==NULL){
        return;
    }

    for(i=0;i<256;i++){
        if(linhas && fscanf(arquivo, "%16s", mem) != EOF){
            strcpy((*memoria)[i].mem, mem);
            (*memoria)[i].instrucao = strtoul(mem, NULL, 2);
        } else {
            strcpy((*memoria)[i].mem, "0000000000000000");
            (*memoria)[i].instrucao = 0;
        }
    }

    fclose(arquivo);
}

// TOTALMENTE EM NCURSES
void imprimeMemorias(instrucao *memoria, int *memDados){
    int opt;
    do {
        clear();
        attron(A_BOLD | COLOR_PAIR(1));
        mvprintw(2, 5, "=== VISUALIZADOR DE MEMÓRIA ===");
        attroff(A_BOLD | COLOR_PAIR(1));

        mvprintw(4, 5, "1. Memoria de Instrucoes");
        mvprintw(5, 5, "2. Memoria de Dados");
        mvprintw(6, 5, "3. Voltar ao Menu Principal");
        mvprintw(8, 5, "Selecione uma opcao: ");
        echo();
        scanw("%d", &opt);
        noecho();

        clear();
        if (opt == 1) {
            attron(A_BOLD | COLOR_PAIR(2));
            mvprintw(1, 20, "--- Memoria de Instrucoes ---");
            attroff(A_BOLD | COLOR_PAIR(2));

            int linha_win = 3;
            for (int linha = 0; linha < 64; linha++) {
                // Coluna 1
                mvprintw(linha_win, 2, "%3d: %16s", linha, memoria[linha].mem);
                // Coluna 2
                mvprintw(linha_win, 25, "%3d: %16s", linha + 64, memoria[linha + 64].mem);
                // Coluna 3
                mvprintw(linha_win, 48, "%3d: %16s", linha + 128, memoria[linha + 128].mem);
                // Coluna 4
                mvprintw(linha_win, 71, "%3d: %16s", linha + 192, memoria[linha + 192].mem);
                linha_win++;
            }
            mvprintw(linha_win + 1, 5, "Pressione qualquer tecla para voltar...");
            refresh();
            getch();
        }
        else if (opt == 2) {
            attron(A_BOLD | COLOR_PAIR(2));
            mvprintw(1, 20, "--- Memoria de Dados ---");
            attroff(A_BOLD | COLOR_PAIR(2));

            int linha_win = 3;
            for (int linha = 0; linha < 64; linha++) {
                mvprintw(linha_win, 2, "%3d: %4d", linha, memDados[linha]);
                mvprintw(linha_win, 18, "%3d: %4d", linha + 64, memDados[linha + 64]);
                mvprintw(linha_win, 34, "%3d: %4d", linha + 128, memDados[linha + 128]);
                mvprintw(linha_win, 50, "%3d: %4d", linha + 192, memDados[linha + 192]);
                linha_win++;
            }
            mvprintw(linha_win + 1, 5, "Pressione qualquer tecla para voltar...");
            refresh();
            getch();
        }
    } while(opt != 3);
}

int8_t extensorBit(int8_t imm){
    imm = imm<<2;
    imm = imm>>2;
    return imm;
}

void run_pipeline(instrucao *memoria, int *bReg, int *pc, int *memDados, registradoresPipeline *pipe, estatInstrucoes *estatInst)
{
    int acabou = 0;
    while(!acabou)
    {
        step_pipeline(memoria, bReg, pc, memDados, pipe, estatInst);

        if(pipe->regIF_ID_atual.inst.instrucao == 0 &&
           eh_bolha(pipe->regID_EX_atual.sinais) &&
           eh_bolha(pipe->regEX_MEM_atual.sinais) &&
           eh_bolha(pipe->regMEM_WB_atual.sinais) &&
           (*pc >= 256 || memoria[*pc].instrucao == 0)) {
            acabou = 1;
        }
    }
}

void step_pipeline(instrucao *memoria, int *bReg, int *pc, int *memDados, registradoresPipeline *pipe, estatInstrucoes *estatInst) {
    executaWB(&pipe->regMEM_WB_atual, bReg, estatInst);
    do_MEM(&pipe->regEX_MEM_atual, &pipe->regMEM_WB_novo, memDados);
    do_EX(&pipe->regID_EX_atual, &pipe->regEX_MEM_novo);
    do_ID(&pipe->regID_EX_novo, &pipe->regIF_ID_atual, bReg);

    int hazard = unidadeDetecHazards(&pipe->regIF_ID_atual, &pipe->regID_EX_atual, &pipe->regEX_MEM_atual);

    if(hazard==1){
        insereStall(&pipe->regID_EX_novo.sinais);
        pipe->regIF_ID_novo = pipe->regIF_ID_atual;
        estatInst->stalls++;
        (*pc)--;
    }
    else if(hazard==2){
        insereFlush(pipe);
    }

    do_IF(&pipe->regIF_ID_novo, memoria, pc);
    atualiza_regs_pipeline(pipe);
    estatInst->ciclos++;

    print_pipeline_state(pipe, estatInst->ciclos);
}

void atualiza_regs_pipeline(registradoresPipeline *pipe) {
    pipe->regIF_ID_atual = pipe->regIF_ID_novo;
    pipe->regID_EX_atual = pipe->regID_EX_novo;
    pipe->regEX_MEM_atual = pipe->regEX_MEM_novo;
    pipe->regMEM_WB_atual = pipe->regMEM_WB_novo;
}

void programCounter(int *pc, sinaisUC *sinais, instrucao *instrucao, int zero){
    if(sinais->jump == 1){
        *pc = instrucao->addr;
        return;
    }
    if(sinais->branch == 1 && zero == 1){
        *pc = *pc + instrucao->imm + 1;
        return;
    }
    (*pc)++;
}

void decodificaInst(instrucao *instrucao){
    instrucao->opcode = instrucao->instrucao >> 12;

    switch(instrucao->opcode){
    case 0:
        instrucao->tipoInst = tipoR;
        instrucao->rs = (instrucao->instrucao >> 9) & 0x7;
        instrucao->rt = (instrucao->instrucao >> 6) & 0x7;
        instrucao->rd = (instrucao->instrucao >> 3) & 0x7;
        instrucao->funct = (instrucao->instrucao) & 0x7;
        break;
    case 2:
        instrucao->tipoInst = tipoJ;
        instrucao->addr = (instrucao->instrucao) & 0xFF;
        break;
    default:
        instrucao->tipoInst = tipoI;
        instrucao->rs = (instrucao->instrucao >> 9) & 0x7;
        instrucao->rt = (instrucao->instrucao >> 6) & 0x7;
        instrucao->imm = (instrucao->instrucao) & 0x3F;
        instrucao->imm = extensorBit(instrucao->imm);
    }
}

void unidadeControle(instrucao *instrucao, sinaisUC *sinais){
    switch(instrucao->opcode){
        case 0:
            sinais->RegDst = 1; sinais->EscReg = 1; sinais->UlaFonte = 0;
            sinais->ulaOp = instrucao->funct; sinais->EscMem = 0;
            sinais->MemParaReg = 1; sinais->jump = 0; sinais->branch = 0;
            break;
        case 2:
            sinais->RegDst = 0; sinais->EscReg = 0; sinais->UlaFonte = 0;
            sinais->ulaOp = 0; sinais->EscMem = 0; sinais->MemParaReg = 0;
            sinais->jump = 1; sinais->branch = 0;
            break;
        case 4:
            sinais->RegDst = 0; sinais->EscReg = 1; sinais->UlaFonte = 1;
            sinais->ulaOp = 0; sinais->EscMem = 0; sinais->MemParaReg = 1;
            sinais->jump = 0; sinais->branch = 0;
            break;
        case 8:
            sinais->RegDst = 0; sinais->EscReg = 0; sinais->UlaFonte = 0;
            sinais->ulaOp = 2; sinais->EscMem = 0; sinais->MemParaReg = 0;
            sinais->jump = 0; sinais->branch = 1;
            break;
        case 11:
            sinais->RegDst = 0; sinais->EscReg = 1; sinais->UlaFonte = 1;
            sinais->ulaOp = 0; sinais->EscMem = 0; sinais->MemParaReg = 0;
            sinais->jump = 0; sinais->branch = 0;
            break;
        case 15:
            sinais->RegDst = 0; sinais->EscReg = 0; sinais->UlaFonte = 1;
            sinais->ulaOp = 0; sinais->EscMem = 1; sinais->MemParaReg = 0;
            sinais->jump = 0; sinais->branch = 0;
            break;
    }
}

int *inicializaBReg(){
    return calloc(8, sizeof(int));
}

void lerRegistradores(int *reg, int8_t rs, int8_t rt, int8_t *valRs, int8_t *valRt){
    *valRs = reg[rs];
    *valRt = reg[rt];
}

void escreveRegistrador(int *reg, int8_t rd, int8_t valor, int EscReg){
    if(EscReg){
        reg[rd] = valor;
    }
}

// IMAGEM EM NCURSES NO MENU PRINCIPAL
void imprimeBancoRegistradores(int *reg){
    attron(A_BOLD | COLOR_PAIR(2));
    mvprintw(3, 50, " Banco de Registradores ");
    attroff(A_BOLD | COLOR_PAIR(2));
    mvprintw(4, 50, "------------------------");
    for(int i=0;i<8;i++){
        mvprintw(5 + i, 50, "   $%d   |   Value: %d  ", i, reg[i]);
    }
    mvprintw(13, 50, "------------------------");
}

int8_t ULA(int op1, int op2, int ulaOp, int *zero, int *overflow){
    int resultado = 0;
    *overflow = 0;
    int8_t res_8bit;

    switch(ulaOp){
        case 0:
            resultado = op1 + op2;
            res_8bit = (int8_t)resultado;
            if(resultado != res_8bit) *overflow = 1;
            break;
        case 2:
            resultado = op1 - op2;
            res_8bit = (int8_t)resultado;
            if(resultado != res_8bit) *overflow = 1;
            break;
        case 4:
            resultado = op1 & op2;
            break;
        case 5:
            resultado = op1 | op2;
            break;
    }

    if(*overflow == 1){
        clear();
        mvprintw(10, 10, "CRITICAL ERROR: OVERFLOW DETECTED!");
        refresh();
        sleep(2);
        exit(1);
    }

    *zero = (resultado == 0) ? 1 : 0;
    return (int8_t)resultado;
}

int *inicializaMemDados(){
    return calloc(256, sizeof(int));
}

void lerMemDados(char *arqMem, int **memDados) {
    arquivoMemDados = fopen(arqMem, "r");
    if (arquivoMemDados == NULL) return;

    for(int i = 0; i < 256; i++) {
        if(fscanf(arquivoMemDados, "%d", &(*memDados)[i]) == EOF) break;
    }
    fclose(arquivoMemDados);
}

void escreveMemDados(int *memDados, int endereco, int8_t valor) {
    if (endereco >= 0 && endereco < 256) {
        memDados[endereco] = valor;
    }
}

int8_t retornaMemoria(int *memDados, uint8_t enderecoULA) {
    return memDados[enderecoULA];
}

void do_IF(IF_ID *out, instrucao *memoria, int *pc) {
    if(*pc >= 256 || memoria[*pc].instrucao == 0) {
        out->inst.instrucao = 0;
        return;
    }
    out->pc = *pc;
    out->inst = memoria[*pc];
    (*pc)++;
}

void do_ID(ID_EX *out, IF_ID *in, int *bReg) {
    decodificaInst(&in->inst);
    unidadeControle(&in->inst, &out->sinais);

    out->rs = in->inst.rs; out->rt = in->inst.rt; out->rd = in->inst.rd;
    out->imm = in->inst.imm; out->funct = in->inst.funct; out->opcode = in->inst.opcode;
    lerRegistradores(bReg, in->inst.rs, in->inst.rt, &out->A, &out->B);
}

void do_EX(ID_EX *in, EX_MEM *out) {
    int op2, zero, overflow;

    if(in->sinais.UlaFonte) op2 = in->imm;
    else op2 = in->B;

    out->ulaSaida = ULA(in->A, op2, in->sinais.ulaOp, &zero, &overflow);
    out->opcode = in->opcode;
    out->B = in->B;
    out->sinais = in->sinais;

    if(in->sinais.RegDst) out->rd = in->rd;
    else out->rd = in->rt;
}

void do_MEM(EX_MEM *in, MEM_WB *out, int *memDados) {
    out->opcode = in->opcode;
    out->rd = in->rd;
    out->sinais = in->sinais;
    out->ulaSaida = in->ulaSaida;

    if(in->sinais.EscMem){
        escreveMemDados(memDados, in->ulaSaida, in->B);
    }

    if(in->sinais.MemParaReg == 0){
        out->mem = retornaMemoria(memDados, in->ulaSaida);
    }
}

// ARMAZENA AS STRINGS GLOBAIS DO CONTEXTO DO PIPELINE PARA O PRINT_STATE EXIBIR
char log_wb_1[100] = {0};
char log_wb_2[100] = {0};

void executaWB(MEM_WB *in, int *bReg, estatInstrucoes *estatInst) {
    if(in->sinais.EscReg == 1) {
        int8_t dadoFinal = (in->sinais.MemParaReg == 1) ? in->mem : in->ulaSaida;

        snprintf(log_wb_1, sizeof(log_wb_1), "[ WB ] Dado %d preparado para escrita ($%d) via %s",
                 dadoFinal, in->rd, (in->sinais.MemParaReg == 1) ? "Memoria" : "ULA");

        escreveRegistrador(bReg, in->rd, dadoFinal, in->sinais.EscReg);

        snprintf(log_wb_2, sizeof(log_wb_2), "[ WB ] Gravado com sucesso no Banco.");
    } else {
        strcpy(log_wb_1, "[ WB ] Nenhuma instrucao gravando no banco neste ciclo.");
        strcpy(log_wb_2, "");
    }
    estatInst->total++;
}

void insereStall(sinaisUC *sinais){
    sinais->EscMem = 0; sinais->EscReg = 0; sinais->IncPC = 0;
    sinais->branch = 0; sinais->MemParaReg = 0; sinais->RegDst = 0;
    sinais->UlaFonte = 0; sinais->ulaOp = 0; sinais->jump = 0;
}

void insereFlush(registradoresPipeline *pipe){
    insereStall(&pipe->regID_EX_novo.sinais);
    pipe->regIF_ID_novo.inst.instrucao = 0;
}

void salvaASM(instrucao *memoria, int linhas) {
    int pc = 0;
    char nomeASM[50]={0}, nome[20], extensao[] = ".asm";

    clear();
    mvprintw(3, 5, "Salvar arquivo ASM (Digite apenas o nome): ");
    echo();
    scanw("%s", nome);
    noecho();

    snprintf(nomeASM, sizeof(nomeASM), "%s%s", nome, extensao);

    arquivo = fopen(nomeASM, "w");
    if (arquivo == NULL) return;

    while(pc < linhas){
        if(memoria[pc].decodificado==0) decodificaInst(&memoria[pc]);

        switch(memoria[pc].opcode){
            case 0:
                if(memoria[pc].funct==0)      fprintf(arquivo,"add $%d, $%d, $%d\n", memoria[pc].rd, memoria[pc].rs, memoria[pc].rt);
                else if(memoria[pc].funct==2) fprintf(arquivo,"sub $%d, $%d, $%d\n", memoria[pc].rd, memoria[pc].rs, memoria[pc].rt);
                else if(memoria[pc].funct==4) fprintf(arquivo,"and $%d, $%d, $%d\n", memoria[pc].rd, memoria[pc].rs, memoria[pc].rt);
                else if(memoria[pc].funct==5) fprintf(arquivo,"or $%d, $%d, $%d\n", memoria[pc].rd, memoria[pc].rs, memoria[pc].rt);
                break;
            case 2:  fprintf(arquivo,"j %d\n", memoria[pc].addr); break;
            case 4:  fprintf(arquivo,"addi $%d, $%d, %d\n", memoria[pc].rt, memoria[pc].rs, memoria[pc].imm); break;
            case 8:  fprintf(arquivo,"beq $%d, $%d, %d\n", memoria[pc].rs, memoria[pc].rt, memoria[pc].imm); break;
            case 11: fprintf(arquivo,"lw $%d, %d($%d)\n", memoria[pc].rt, memoria[pc].imm, memoria[pc].rs); break;
            case 15: fprintf(arquivo,"sw $%d, %d($%d)\n", memoria[pc].rt, memoria[pc].imm, memoria[pc].rs); break;
        }
        pc++;
    }
    fclose(arquivo);
    mvprintw(6, 5, "Arquivo '%s' salvo com sucesso! Pressione algo...", nomeASM);
    getch();
}

void salvaDAT(int *memDados){
    char nomeDAT[50]={0}, nome[20], extensao[] = ".dat";

    clear();
    mvprintw(3, 5, "Salvar arquivo DATA (Digite apenas o nome): ");
    echo();
    scanw("%s", nome);
    noecho();

    snprintf(nomeDAT, sizeof(nomeDAT), "%s%s", nome, extensao);

    arquivo = fopen(nomeDAT, "w");
    if (arquivo == NULL) return;

    for(int i=0;i<256;i++){
        fprintf(arquivo,"%d\n",memDados[i]);
    }
    fclose(arquivo);
    mvprintw(6, 5, "Arquivo '%s' salvo com sucesso! Pressione algo...", nomeDAT);
    getch();
}

void contabilizaEstat(instrucao *memoria, estatInstrucoes *estat, int pc){
    switch(memoria[pc].tipoInst){
        case 0:
            switch(memoria[pc].opcode){
                case 4:  (*estat).addi++; break;
                case 8:  (*estat).beq++;  break;
                case 11: (*estat).lw++;   break;
                case 15: (*estat).sw++;   break;
            }
            (*estat).tipoI++;
            break;
        case 1:
            (*estat).j++; (*estat).tipoJ++;
            break;
        case 2:
            switch(memoria[pc].funct){
                case 0: (*estat).add++; break;
                case 2: (*estat).sub++; break;
                case 4: (*estat).and++; break;
                case 5: (*estat).or++;  break;
            }
            (*estat).tipoR++;
            break;
    }
}

void imprimeEstatistica(estatInstrucoes estatInst){
    estatInst.CPI = 0;
    if(estatInst.total > 0){
        estatInst.CPI = (float)estatInst.ciclos / (float)estatInst.total;
    }

    clear();
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(1, 5, "+------------------------------------------------+");
    mvprintw(2, 5, "|           ESTATISTICAS DO PIPELINE             |");
    mvprintw(3, 5, "+------------------------------------------------+");
    attroff(COLOR_PAIR(1) | A_BOLD);

    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(5, 5, "[ Metricas Globais ]");
    attroff(COLOR_PAIR(2) | A_BOLD);

    mvprintw(7, 5, "Total executadas: %4d", estatInst.total);
    mvprintw(8, 5, "Ciclos: %4d",   estatInst.ciclos);
    mvprintw(9, 5, "CPI total:        %4.2f", estatInst.CPI);
    mvprintw(10, 5, "Stalls ocorrido:  %4d", estatInst.stalls);

    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(12, 5, "[ Divisao por Tipo ]");
    attroff(COLOR_PAIR(2) | A_BOLD);

    mvprintw(14, 5, "Tipo R: %3d | Tipo I: %3d | Tipo J: %3d", estatInst.tipoR, estatInst.tipoI, estatInst.tipoJ);

    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(16, 5, "[ Detalhamento das Instrucoes ]");
    attroff(COLOR_PAIR(2) | A_BOLD);

    mvprintw(18, 5, "ADD: %3d | SUB: %3d | AND: %3d | OR: %3d", estatInst.add, estatInst.sub, estatInst.and, estatInst.or);
    mvprintw(19, 5, "ADDI: %3d | BEQ: %3d | LW: %3d | SW: %3d", estatInst.addi, estatInst.beq, estatInst.lw, estatInst.sw);
    mvprintw(20, 5, "J: %3d", estatInst.j);

    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(22, 5, "+------------------------------------------------+");
    mvprintw(23, 5, "|      Pressione qualquer tecla para voltar      |");
    mvprintw(24, 5, "+------------------------------------------------+");
    attroff(COLOR_PAIR(1) | A_BOLD);

    refresh();
    getch();
}

void salvaEstado(historico *hist, int pc, int *memDados, int *bReg, estatInstrucoes *estatInst){
    if(hist->topo >= MAX_HIST) return;
    estado *e = &hist->estados[hist->topo];
    e->pc = pc;

    for(int i=0;i<256;i++) e->memDados[i] = memDados[i];
    for(int i=0;i<8;i++)   e->bReg[i] = bReg[i];

    e->estat = *estatInst;
    hist->topo++;
}

void voltaInstrucao(historico *hist, int *pc, int *memDados, int *bReg, estatInstrucoes *estatInst){
    if(hist->topo <= 0) return;

    hist->topo--;
    estado *e = &hist->estados[hist->topo];
    *pc = e->pc;

    for(int i=0;i<256;i++) memDados[i] = e->memDados[i];
    for(int i=0;i<8;i++)   bReg[i] = e->bReg[i];

    *estatInst = e->estat;
}

void imprimeInstrucao(instrucao *memoria, int pc) {
    if(memoria[pc].decodificado==0){
        decodifica(&memoria[pc]);
    }
}

void decodifica(instrucao *instrucao){
    instrucao->opcode = instrucao->instrucao >> 12;

    switch(instrucao->opcode){
    case 0:
        instrucao->tipoInst = tipoR;
        instrucao->rs = (instrucao->instrucao >> 9) & 0x7;
        instrucao->rt = (instrucao->instrucao >> 6) & 0x7;
        instrucao->rd = (instrucao->instrucao >> 3) & 0x7;
        instrucao->funct = (instrucao->instrucao) & 0x7;
        break;
    case 2:
        instrucao->tipoInst = tipoJ;
        instrucao->addr = (instrucao->instrucao) & 0xFF;
        break;
    default:
        instrucao->tipoInst = tipoI;
        instrucao->rs = (instrucao->instrucao >> 9) & 0x7;
        instrucao->rt = (instrucao->instrucao >> 6) & 0x7;
        instrucao->imm = (instrucao->instrucao) & 0x3F;
        instrucao->imm = extensorBit(instrucao->imm);
    }
}

int eh_bolha(sinaisUC sinais) {
    if ((sinais.EscReg || sinais.EscMem || sinais.branch || sinais.jump)) {
        return 0;
    }
    return 1;
}

void print_pipeline_state(registradoresPipeline *pipe, int ciclo) {
    clear();

    attron(A_BOLD | COLOR_PAIR(1));
    mvprintw(1, 5, "================ PIPELINE STATE - Ciclo %d ================", ciclo);
    attroff(A_BOLD | COLOR_PAIR(1));

    int ocupados = 0;

    // IF
    mvprintw(3, 5, "[IF]  -> ");
    if(pipe->regIF_ID_atual.inst.instrucao != 0) {
        printw("PC=%d | Opcode=%d", pipe->regIF_ID_atual.pc, pipe->regIF_ID_atual.inst.opcode);
        ocupados++;
    } else {
        attron(COLOR_PAIR(4)); printw("Stall / NOP (Vazio)"); attroff(COLOR_PAIR(4));
    }

    // ID
    mvprintw(5, 5, "[ID]  -> ");
    if(!eh_bolha(pipe->regID_EX_atual.sinais)) {
        printw("Opcode=%d | rs=$%d rt=$%d rd=$%d | A=%d B=%d",
               pipe->regID_EX_atual.opcode, pipe->regID_EX_atual.rs, pipe->regID_EX_atual.rt,
               pipe->regID_EX_atual.rd, pipe->regID_EX_atual.A, pipe->regID_EX_atual.B);
        ocupados++;
    } else {
        attron(COLOR_PAIR(4)); printw("Stall / NOP"); attroff(COLOR_PAIR(4));
    }

    // EX
    mvprintw(7, 5, "[EX]  -> ");
    if(!eh_bolha(pipe->regEX_MEM_atual.sinais)) {
        printw("Opcode=%d | Saida ULA=%d | Reg Destino=$%d",
               pipe->regEX_MEM_atual.opcode, pipe->regEX_MEM_atual.ulaSaida, pipe->regEX_MEM_atual.rd);
        ocupados++;
    } else {
        attron(COLOR_PAIR(4)); printw("Stall / NOP"); attroff(COLOR_PAIR(4));
    }

    // MEM
    mvprintw(9, 5, "[MEM] -> ");
    if(!eh_bolha(pipe->regMEM_WB_atual.sinais)) {
        printw("Opcode=%d | Endereço=%d | Lido=%d | Target=$%d",
               pipe->regMEM_WB_atual.opcode, pipe->regMEM_WB_atual.ulaSaida,
               pipe->regMEM_WB_atual.mem, pipe->regMEM_WB_atual.rd);
    } else {
        attron(COLOR_PAIR(4)); printw("Stall / NOP"); attroff(COLOR_PAIR(4));
    }

    // Linha informativa de Writeback coletada de executaWB
    attron(COLOR_PAIR(2));
    mvprintw(12, 5, "%s", log_wb_1);
    if(strlen(log_wb_2) > 0) mvprintw(13, 5, "%s", log_wb_2);
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(3));
    mvprintw(15, 5, "Taxa de Ocupacao: %d/4 estágios ativos.", ocupados);
    attroff(COLOR_PAIR(3));

    mvprintw(17, 5, "==========================================================");
    mvprintw(19, 5, "Pressione qualquer tecla para o proximo ciclo...");

    refresh();
    getch(); // Executa ciclo por ciclo a cada clique de tecla no modo Step
}
