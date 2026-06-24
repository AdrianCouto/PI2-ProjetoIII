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
        //printf("\nAcesso negado!\n");
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

// Leitura da memória
int lerMem(int colunaspainel, int linhaspainel, char *arq, instrucao **memoria, int linhas){
    
    *memoria = calloc(256, sizeof(instrucao));
    
    clear();

    printBorda(linhaspainel, colunaspainel);

if(*memoria == NULL){
    mvprintw((linhaspainel/2) , (colunaspainel/2), "Memoria nao carregada!");
    refresh();
    return 0;
}

arquivo = fopen(arq, "r");

if(arquivo == NULL){
    mvprintw((linhaspainel/2) , (colunaspainel/2), "Arquivo nao encontrado!");
    refresh();
    return 0;
}

    int i=0;
    char mem[17];

    for(i=0;i<256;i++){
        if(linhas && fscanf(arquivo, "%16s", mem) != EOF){
            strcpy((*memoria)[i].mem, mem);
            (*memoria)[i].instrucao = strtoul(mem, NULL, 2);
        } else {
            strcpy((*memoria)[i].mem, "0000000000000000");
            (*memoria)[i].instrucao = 0;
        }
    }
    
    refresh();

    fclose(arquivo);
    return 1;

}

int lerMemDados(int linhaspainel, int colunaspainel, char *arqMem, int **memDados) {
    
    clear();

    printBorda(linhaspainel, colunaspainel);
    
    int i=0;

    if (*memDados == NULL) {
        mvprintw((linhaspainel/2) , (colunaspainel/2), "Erro ao alocar memória");
        return 0;
    }

    arquivoMemDados = fopen(arqMem, "r");
    if (arquivoMemDados == NULL) {
        mvprintw((linhaspainel/2) , (colunaspainel/2), "Erro ao abrir o arquivo %s", arqMem);
        return 0;
    }

    for(i = 0; i < 256; i++) {
        fscanf(arquivoMemDados, "%d", &(*memDados)[i]);
    }

    //printf("\nMemória carregada!\n");

    fclose(arquivoMemDados);
    return 1;
}

void imprimeMemorias(int colunaspainel, int linhaspainel, instrucao *memoria, int *memDados){
    
    int selecionado = 0;
    int tecla;

    char *opcoes[] = {
        "Memoria de Instruções",
        "Memoria de Dados"
    };

    while(1){

        clear();
        printBorda(linhaspainel, colunaspainel);

        mvprintw((linhaspainel/2) - 2, (colunaspainel/2) - 10, "Escolha uma memoria");

        for(int i = 0; i < 2; i++){

            if(i == selecionado){
                attron(A_REVERSE | A_BOLD);
            }
            
            mvprintw(linhaspainel/2 + i, colunaspainel/2 - 10, "%s", opcoes[i]);
            
            if(i == selecionado){
                attroff(A_REVERSE | A_BOLD);
            }
        }

        refresh();

        tecla = getch();

        switch(tecla){

            case KEY_UP:
                selecionado--;
                if(selecionado < 0)
                    selecionado = 1;
                break;

            case KEY_DOWN:
                selecionado++;
                if(selecionado > 1)
                    selecionado = 0;
                break;

            case 10:
            
                if(selecionado == 0){
                    clear();

                    printBorda(linhaspainel, colunaspainel);
                    
                    attron(A_BOLD);
                    mvprintw(2, (colunaspainel/2) - 10, "Memoria de Instrucoes");
                    attroff(A_BOLD);

                    for(int linha = 0; linha < 32; linha++){

                        mvprintw(linha + 4,   2, "%3d: %16s", linha,       memoria[linha].mem);
                        mvprintw(linha + 4,  25, "%3d: %16s", linha + 32,  memoria[linha + 32].mem);
                        mvprintw(linha + 4,  48, "%3d: %16s", linha + 64,  memoria[linha + 64].mem);
                        mvprintw(linha + 4,  71, "%3d: %16s", linha + 96,  memoria[linha + 96].mem);
                        mvprintw(linha + 4,  94, "%3d: %16s", linha + 128, memoria[linha + 128].mem);
                        mvprintw(linha + 4, 117, "%3d: %16s", linha + 160, memoria[linha + 160].mem);
                        mvprintw(linha + 4, 140, "%3d: %16s", linha + 192, memoria[linha + 192].mem);
                        mvprintw(linha + 4, 163, "%3d: %16s", linha + 224, memoria[linha + 224].mem);
                    }

                    refresh();
                    getch();
                }

                if(selecionado == 1){
                    clear();

                    printBorda(linhaspainel, colunaspainel);

                    mvprintw(2, (colunaspainel/2), "Memoria de Dados");

                    for(int linha = 0; linha < 32; linha++) {

                        mvprintw(linha + 4,   2, "%3d: %3d", linha,       memDados[linha]);
                        mvprintw(linha + 4,  22, "%3d: %3d", linha + 32,  memDados[linha + 32]);
                        mvprintw(linha + 4,  42, "%3d: %3d", linha + 64,  memDados[linha + 64]);
                        mvprintw(linha + 4,  62, "%3d: %3d", linha + 96,  memDados[linha + 96]);
                        mvprintw(linha + 4,  82, "%3d: %3d", linha + 128, memDados[linha + 128]);
                        mvprintw(linha + 4, 102, "%3d: %3d", linha + 160, memDados[linha + 160]);
                        mvprintw(linha + 4, 122, "%3d: %3d", linha + 192, memDados[linha + 192]);
                        mvprintw(linha + 4, 142, "%3d: %3d", linha + 224, memDados[linha + 224]);
                    }
                    refresh();
                    getch();
                }

                return;
        }
    }
}

int8_t extensorBit(int8_t imm){
    imm = imm<<2;
    //printf("\n%d", imm);      // 111111 = -1  <- 00111111 << 2 -> 111111100 >> 2 -> 11111111

    imm = imm>>2;
    //printf("\n%d", imm);

    return imm;
}

// RUN

void run_pipeline(historico *hist, instrucao *memoria, int *bReg, int *pc, int *memDados, registradoresPipeline *pipe, estatInstrucoes *estatInst, int *hazardTipo){
    while (1){
        *hazardTipo = step_pipeline(hist, memoria, bReg, pc, memDados, pipe, estatInst);

        if ((*pc >= 256 || memoria[*pc].instrucao == 0) && pipe->regIF_ID_atual.inst.instrucao == 0 && pipe->regID_EX_atual.opcode == 0 && pipe->regEX_MEM_atual.opcode == 0 && pipe->regMEM_WB_atual.opcode == 0){
            break;
        }
    }
    usleep(150000);
}

int step_pipeline(historico *hist, instrucao *memoria, int *bReg, int *pc, int *memDados, registradoresPipeline *pipe, estatInstrucoes *estatInst) {    
    salvaEstado(hist, *pc, memDados, bReg, estatInst, pipe);

    
    int instrucao_valida = (pipe->regMEM_WB_atual.opcode != 0) || (pipe->regMEM_WB_atual.opcode == 0 && pipe->regMEM_WB_atual.rd != 0);

    if (instrucao_valida && 
       (pipe->regMEM_WB_atual.sinais.EscReg || 
        pipe->regMEM_WB_atual.opcode == 8 ||   // beq
        pipe->regMEM_WB_atual.opcode == 15 ||  // sw
        pipe->regMEM_WB_atual.opcode == 2)) {  // j
    
        estatInst->total++;
        
        if (pipe->regMEM_WB_atual.opcode == 0) {
            estatInst->tipoR++;
            if (pipe->regMEM_WB_atual.sinais.ulaOp == 0) estatInst->add++;
            else if (pipe->regMEM_WB_atual.sinais.ulaOp == 2) estatInst->sub++;
            else if (pipe->regMEM_WB_atual.sinais.ulaOp == 4) estatInst->and++;
            else if (pipe->regMEM_WB_atual.sinais.ulaOp == 5) estatInst->or++;
        } else if (pipe->regMEM_WB_atual.opcode == 2) {
            estatInst->tipoJ++;
            estatInst->j++;
        } else {
            estatInst->tipoI++;
            if (pipe->regMEM_WB_atual.opcode == 4) estatInst->addi++;
            else if (pipe->regMEM_WB_atual.opcode == 8) estatInst->beq++;
            else if (pipe->regMEM_WB_atual.opcode == 11) estatInst->lw++;
            else if (pipe->regMEM_WB_atual.opcode == 15) estatInst->sw++;
        }
    }

    // Executa os estágios normais
    Executa_WB(&pipe->regMEM_WB_atual, bReg);
    Executa_MEM(&pipe->regEX_MEM_atual, &pipe->regMEM_WB_novo, memDados);
    Executa_EX(&pipe->regID_EX_atual, &pipe->regEX_MEM_novo, &pipe->regMEM_WB_atual);
    Executa_ID(&pipe->regID_EX_novo, &pipe->regIF_ID_atual, bReg);

    tipoHazard hazard = unidadeDetecHazards(&pipe->regIF_ID_atual, &pipe->regID_EX_atual, &pipe->regEX_MEM_atual);
    
    if(hazard == hazardDados) { 
        insereStall(pipe, estatInst);
    }
    else if(hazard == hazardControle)
    {
    if(pipe->regID_EX_atual.sinais.jump)
        *pc = pipe->regID_EX_atual.addr;

    else if(pipe->regEX_MEM_atual.sinais.branch)
        *pc = pipe->regEX_MEM_atual.pc + pipe->regEX_MEM_atual.imm;

    insereFlush(pipe);
    }

    // 3. BUSCA A PRÓXIMA INSTRUÇÃO
    Executa_IF(&pipe->regIF_ID_novo, memoria, pc);
    
    // 4. ATUALIZA OS VALORES DE "NOVO" PARA "ATUAL" PARA O PRÓXIMO CICLO
    atualiza_regs_pipeline(pipe);
    
    // 5. ATUALIZA CONTADORES DE CICLO E CPI
    estatInst->ciclos++;
    if (estatInst->total > 0) {
        estatInst->CPI = (float)estatInst->ciclos / estatInst->total;
    } else {
        estatInst->CPI = 0.0;
    }
    return hazard;
}

void atualiza_regs_pipeline(registradoresPipeline *pipe)
{
    if(!pipe->ctrl.stallIF)
        pipe->regIF_ID_atual = pipe->regIF_ID_novo;

    if(pipe->ctrl.flushIF)
        memset(&pipe->regIF_ID_atual, 0, sizeof(IF_ID));
    if(pipe->ctrl.flushID)
        memset(&pipe->regID_EX_atual, 0, sizeof(ID_EX));
    else
        pipe->regID_EX_atual = pipe->regID_EX_novo;

    pipe->regEX_MEM_atual = pipe->regEX_MEM_novo;
    pipe->regMEM_WB_atual = pipe->regMEM_WB_novo;

    pipe->ctrl.flushIF = 0;
    pipe->ctrl.flushID = 0;
    pipe->ctrl.stallIF = 0;
    pipe->ctrl.stallID = 0;
}

// Decodificação
void decodificaInst(instrucao *instrucao){

    instrucao->opcode = instrucao->instrucao >> 12; // Pega os 4 bits do opcode

    switch(instrucao->opcode){
    case 0:
        instrucao->tipoInst = tipoR;
        instrucao->rs = (instrucao->instrucao >> 9) & 0x7; // pega os 3 bits do rs (desloca 6 bits para a direita e pega os 3 mais significativos que ficaram)
        instrucao->rt = (instrucao->instrucao >> 6) & 0x7; // pega os 3 bits do rt
        instrucao->rd = (instrucao->instrucao >> 3) & 0x7; // pega os 3 bits do rd
        instrucao->funct = (instrucao->instrucao) & 0x7;
        //printf("\n[ Tipo R ] \n");
        //printf("opcode: %d\n", instrucao->opcode);
        //printf("rs: %d\n", instrucao->rs);
        //printf("rt: %d\n", instrucao->rt);
        //printf("rd: %d\n", instrucao->rd);
        //printf("funct: %d\n", instrucao->funct);
        break;

    case 2:
        instrucao->tipoInst = tipoJ;
        instrucao->addr = (instrucao->instrucao) &0xFF; // pega os 8 bits do adress
        //printf("\n[ Tipo J ]\n");
        //printf("opcode: %d\n", instrucao->opcode);
        //printf("address: %d\n", instrucao->addr);
        break;

    default:
        instrucao->tipoInst = tipoI;
        instrucao->rs = (instrucao->instrucao >> 9) &0x7; // pega os 3 bits do rs
        instrucao->rt = (instrucao->instrucao >> 6) &0x7; // pega os 3 bits do rt
        instrucao->imm = (instrucao->instrucao) &0x3F; // pega os 6 bits do imediato (deve passar por um extensor antes da ULA)
        instrucao->imm = extensorBit(instrucao->imm);
        //printf("\n[ Tipo I ] \n");
        //printf("opcode: %d\n", instrucao->opcode);
        //printf("rs: %d\n", instrucao->rs);
        //printf("rt: %d\n", instrucao->rt);
        //printf("imediato: %d\n", instrucao->imm);
    }
}

//UC
void unidadeControle(instrucao *instrucao, sinaisUC *sinais){
    switch(instrucao->opcode){
        case 0: // opcode = 0000
            sinais->RegDst = 1;
            sinais->EscReg = 1;
            sinais->UlaFonte = 0;
            sinais->ulaOp = instrucao->funct;
            sinais->EscMem = 0;
            sinais->MemParaReg = 0;
            sinais->jump = 0;
            sinais->branch = 0;

            break;

        case 2: // opcode = 0010 - J
            sinais->RegDst = 0;
            sinais->EscReg = 0;
            sinais->UlaFonte = 0;
            sinais->ulaOp = 0;
            sinais->EscMem = 0;
            sinais->MemParaReg = 0;
            sinais->jump = 1;
            sinais->branch = 0;

            break;

        case 4: // opcode = 0100 - Addi
            sinais->RegDst = 0;
            sinais->EscReg = 1;
            sinais->UlaFonte = 1;
            sinais->ulaOp = 0;
            sinais->EscMem = 0;
            sinais->MemParaReg = 0;
            sinais->jump = 0;
            sinais->branch = 0;

            break;

        case 8: // opcode = 1000 - BEQ 
            sinais->RegDst = 0;
            sinais->EscReg = 0;
            sinais->UlaFonte = 0;
            sinais->ulaOp = 2; // SUB
            sinais->EscMem = 0;
            sinais->MemParaReg = 0;
            sinais->jump = 0;
            sinais->branch = 1;

            break;

        case 11: // opcode = 1011 - lw // add
            sinais->RegDst = 0;
            sinais->EscReg = 1;
            sinais->UlaFonte = 1;
            sinais->ulaOp = 0; // add
            sinais->EscMem = 0;
            sinais->MemParaReg = 1;
            sinais->jump = 0;
            sinais->branch = 0;
            
            break;

        case 15: // opcode = 1111 - sw
            sinais->RegDst = 0;
            sinais->EscReg = 0;
            sinais->UlaFonte = 1;
            sinais->ulaOp = 0; // add
            sinais->EscMem = 1;
            sinais->MemParaReg = 0;
            sinais->jump = 0;
            sinais->branch = 0;

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


void imprimeBancoRegistradores(int *reg){
    clear();
    attron(A_BOLD | COLOR_PAIR(2));
    mvprintw((LINES-10)/2-1, (COLS-19)/2, "BANCO DE REGISTRADORES");
    attroff(A_BOLD | COLOR_PAIR(2));
    for(int i = 0; i < 8; i++) {
        mvprintw((LINES-10)/2 + i, (COLS-6)/2, "$%d: %d", i, reg[i]);
    }
    getch();
}

int8_t ULA(int op1, int op2, int ulaOp, int *zero, int *overflow){
    int resultado = 0;
    *overflow = 0;
    int8_t res_8bit;

    switch(ulaOp){
        case 0: // ADD, LW/SW , ADDI
            resultado = op1 + op2;

                res_8bit = (int8_t)resultado;

                if(resultado != res_8bit){
                    *overflow = 1;
                }
            break;

        case 2: // SUB, BEQ
            resultado = op1 - op2;

                res_8bit = (int8_t)resultado;

                if(resultado != res_8bit){
                    *overflow = 1;
                }
            break;

        case 4: // AND
            resultado = op1 & op2;
            break;

        case 5: // OR
            resultado = op1 | op2;
            break;

        default:
            //printf("\nOperação da ULA inválida!\n");
    }
    //FECHA SE FOR OVERFLOW
    if(*overflow == 1){
    //printf("\n OVERFLOW!\n");
    exit(1);
}

    // flag zero
    if(resultado == 0){
        *zero = 1;
    } else {
        *zero = 0;
    }

    return resultado;
}

int *inicializaMemDados(){
    return calloc(256, sizeof(int));
}

void escreveMemDados(int *memDados, int endereco, int8_t valor) {
    if (endereco >= 0 && endereco < 256) {
        memDados[endereco] = valor;
    } else {
        //printf("\nErro ao escrever na memória.\n");
    }
}

int8_t retornaMemoria(int *memDados, uint8_t enderecoULA) {
    return memDados[enderecoULA];
}

void Executa_IF(IF_ID *IF_ID, instrucao *memoria, int *pc) {
    if(*pc >= 256 || memoria[*pc].instrucao == 0){
        memset(IF_ID,0,sizeof(*IF_ID));
        return;
    }

    IF_ID->pc = *pc;
    IF_ID->inst = memoria[*pc];

    (*pc)++;

}

void Executa_ID(ID_EX *ID_EX, IF_ID *IF_ID, int *bReg) {

    if(IF_ID->inst.instrucao == 0){
        memset(ID_EX, 0, sizeof(*ID_EX));
        return;
    }

    decodificaInst(&IF_ID->inst);
    unidadeControle(&IF_ID->inst, &ID_EX->sinais);

    ID_EX->rs = IF_ID->inst.rs;
    ID_EX->rt = IF_ID->inst.rt;
    ID_EX->rd = IF_ID->inst.rd;
    ID_EX->imm = IF_ID->inst.imm;
    ID_EX->addr = IF_ID->inst.addr;
    ID_EX->pc = IF_ID->pc;
    ID_EX->funct = IF_ID->inst.funct;
    ID_EX->opcode = IF_ID->inst.opcode;

    lerRegistradores(bReg, IF_ID->inst.rs, IF_ID->inst.rt, &ID_EX->A, &ID_EX->B);
}

void Executa_EX(ID_EX *ID_EX, EX_MEM *EX_MEM, MEM_WB *MEM_WB_atual) {
    if (ID_EX->opcode == 0 && ID_EX->rd == 0 && ID_EX->rs == 0 && ID_EX->rt == 0) {
        EX_MEM->opcode = 0;
        EX_MEM->rd = 0;
        EX_MEM->ulaSaida = 0;
        EX_MEM->sinais.EscReg = 0;
        EX_MEM->sinais.branch = 0;
        EX_MEM->sinais.jump = 0;
        return;
    }

    EX_MEM->imm = ID_EX->imm;
    EX_MEM->pc = ID_EX->pc;
    
    uint8_t forwardA = 0, forwardB = 0;

    forwardingUnit(ID_EX, EX_MEM, MEM_WB_atual, &forwardA, &forwardB);

    int valA;

    if (forwardA == 2) {
        valA = EX_MEM->ulaSaida; 
    } else if (forwardA == 1) {
        valA = MEM_WB_atual->sinais.MemParaReg ? MEM_WB_atual->mem : MEM_WB_atual->ulaSaida;
    } else {
        valA = ID_EX->A;
    }

    int valB_final;
    if (forwardB == 2) {
        valB_final = EX_MEM->ulaSaida;
    } else if (forwardB == 1) {
        valB_final = MEM_WB_atual->sinais.MemParaReg ? MEM_WB_atual->mem : MEM_WB_atual->ulaSaida;
    } else {
        valB_final = ID_EX->B; 
    }

    int op2;
    if (ID_EX->sinais.UlaFonte) {
        op2 = ID_EX->imm;
    } else {
        op2 = valB_final;
    }

    int zero;
    int overflow;

    EX_MEM->ulaSaida = ULA(valA, op2, ID_EX->sinais.ulaOp, &zero, &overflow);

    EX_MEM->zero = zero;
    EX_MEM->opcode = ID_EX->opcode;
    EX_MEM->B = valB_final;
    EX_MEM->sinais = ID_EX->sinais;

    if (ID_EX->sinais.RegDst)
        EX_MEM->rd = ID_EX->rd;
    else
        EX_MEM->rd = ID_EX->rt;

}

void Executa_MEM(EX_MEM *EX_MEM, MEM_WB *MEM_WB,int *memDados){
    if (EX_MEM->opcode == 0 && EX_MEM->rd == 0){
            MEM_WB->opcode = 0;
            MEM_WB->rd = 0;
            MEM_WB->ulaSaida = 0;
            MEM_WB->mem = 0;
            MEM_WB->sinais.EscReg = 0;
        return; // Encerra o estágio mais cedo pois é um NOP 
    }

    MEM_WB->opcode = EX_MEM->opcode;
    MEM_WB->rd = EX_MEM->rd;
    MEM_WB->sinais = EX_MEM->sinais;
    MEM_WB->ulaSaida = EX_MEM->ulaSaida;

    if(EX_MEM->sinais.EscMem){
        escreveMemDados(memDados, EX_MEM->ulaSaida, EX_MEM->B);
    }

    if(EX_MEM->sinais.MemParaReg == 0){
        MEM_WB->mem = retornaMemoria(memDados,EX_MEM->ulaSaida);
    }
}

void Executa_WB(MEM_WB *MEM_WB, int *bReg) { 

    if (MEM_WB->rd == 0) {
        bReg[0] = 0;
        return;
    }
    
    if(MEM_WB->sinais.EscReg == 1) {
        int8_t dadoFinal;
        
        if(MEM_WB->sinais.MemParaReg == 1){
            dadoFinal = MEM_WB->mem;
        }else{
            dadoFinal = MEM_WB->ulaSaida;
        }

        escreveRegistrador(bReg, MEM_WB->rd, dadoFinal, MEM_WB->sinais.EscReg);
    }

}

void insereStall(registradoresPipeline *pipe, estatInstrucoes *estat){
    
    pipe->ctrl.stallIF = 1;
    pipe->ctrl.stallID = 1;

    // mantém a instrução em IF/ID
    pipe->regIF_ID_novo = pipe->regIF_ID_atual;

    // injeta uma bolha em ID/EX
    memset(&pipe->regID_EX_novo, 0, sizeof(ID_EX));

    estat->stalls++;
}

void insereFlush(registradoresPipeline *pipe) {

    memset(&pipe->regIF_ID_novo, 0, sizeof(IF_ID));
    memset(&pipe->regID_EX_novo, 0, sizeof(ID_EX));
}

void ajustarPC(int *pc, int novoPC) {
    *pc = novoPC;
}

void salvaASM(int colunaspainel, int linhaspainel, instrucao *memoria, int linhas)
{
    int pc = 0;
    int tecla;
    int selecionado = 0;
    int indice = 1;

    char nome[20];
    char nomeASM[50];
    char extensao[] = ".asm";

    clear();
    printBorda(linhaspainel, colunaspainel);

    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(linhaspainel/2, colunaspainel/2 - 25, "Digite o nome do arquivo:");
    attroff(COLOR_PAIR(3) | A_BOLD);

    echo();
    curs_set(1);

    mvgetnstr(linhaspainel/2, colunaspainel/2 + 5, nome, sizeof(nome)-1);

    noecho();
    curs_set(0);
    
    snprintf(nomeASM, sizeof(nomeASM), "%s%s", nome, extensao);

    while(access(nomeASM, F_OK) != -1){

        char *opcoes[] = { 
            "Sim (sobrescrever)",
            "Não (criar outro nome)"
        };
        
        int confirmou = 0;

        while(!confirmou){

            clear();
            printBorda(linhaspainel, colunaspainel);

            attron(COLOR_PAIR(4) | A_BOLD);
            mvprintw(linhaspainel/2 - 2, colunaspainel/2 - 25, "Arquivo '%s' ja existe.", nomeASM);
            mvprintw(linhaspainel/2 - 1, colunaspainel/2 - 25, "Deseja sobrescrever?");
            attroff(COLOR_PAIR(4) | A_BOLD);

            for(int i = 0; i < 2; i++){
                if(i == selecionado){
                    attron(A_REVERSE | A_BOLD);
                }
                
                mvprintw(linhaspainel/2 + 2 + i, colunaspainel/2 - 10, "%s", opcoes[i]);

                if(i == selecionado){
                    attroff(A_REVERSE | A_BOLD);
                }
            }

            refresh();
            tecla = getch();

            switch(tecla){
                case KEY_UP:
                    selecionado--;
                    if(selecionado < 0)
                        selecionado = 1;
                    break;

                case KEY_DOWN:
                    selecionado++;
                    if(selecionado > 1)
                        selecionado = 0;
                    break;

                case 10:       
                case KEY_ENTER:   
                    confirmou = 1;
                    break;
            }
        }

        if(selecionado == 0){ 
            break; 
        } else {
            snprintf(nomeASM, sizeof(nomeASM), "%s_%d%s", nome, indice++, extensao);
        }
    }

    // Código de escrita do arquivo (Mantido igual, removido apenas o goto desnecessário)
    FILE *arquivo = fopen(nomeASM, "w");

    if(arquivo == NULL){
        clear();
        printBorda(linhaspainel, colunaspainel);

        attron(COLOR_PAIR(4) | A_BOLD);
        mvprintw(linhaspainel/2, colunaspainel/2 - 10, "Erro ao criar arquivo!");
        attroff(COLOR_PAIR(4) | A_BOLD);

        refresh();
        getch();
        return;
    }

    while(pc < linhas){
        if(memoria[pc].decodificado == 0)
            decodificaInst(&memoria[pc]);

        switch(memoria[pc].opcode){
            case 0:
                if(memoria[pc].funct == 0)
                    fprintf(arquivo, "add $%d, $%d, $%d\n", memoria[pc].rd, memoria[pc].rs, memoria[pc].rt);
                else if(memoria[pc].funct == 2)
                    fprintf(arquivo, "sub $%d, $%d, $%d\n", memoria[pc].rd, memoria[pc].rs, memoria[pc].rt);
                else if(memoria[pc].funct == 4)
                    fprintf(arquivo, "and $%d, $%d, $%d\n", memoria[pc].rd, memoria[pc].rs, memoria[pc].rt);
                else if(memoria[pc].funct == 5)
                    fprintf(arquivo, "or $%d, $%d, $%d\n", memoria[pc].rd, memoria[pc].rs, memoria[pc].rt);
                break;

            case 2:
                fprintf(arquivo, "j %d\n", memoria[pc].addr);
                break;
            case 4:
                fprintf(arquivo, "addi $%d, $%d, %d\n", memoria[pc].rt, memoria[pc].rs, memoria[pc].imm);
                break;
            case 8:
                fprintf(arquivo, "beq $%d, $%d, %d\n", memoria[pc].rs, memoria[pc].rt, memoria[pc].imm);
                break;
            case 11:
                fprintf(arquivo, "lw $%d, %d($%d)\n", memoria[pc].rt, memoria[pc].imm, memoria[pc].rs);
                break;
            case 15:
                fprintf(arquivo,"sw $%d, %d($%d)\n", memoria[pc].rt, memoria[pc].imm, memoria[pc].rs);
                break;
        }
        pc++;
    }

    fclose(arquivo);

    clear();
    printBorda(linhaspainel, colunaspainel);

    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(linhaspainel/2, colunaspainel/2 - 20, "Arquivo '%s' salvo com sucesso!", nomeASM);
    attroff(COLOR_PAIR(3) | A_BOLD);

    refresh();
    getch();
}

void salvaDAT(int *memDados){
    char nomeDAT[50]={0}, nome[20], extensao[] = ".dat", resposta;

    fgets(nome, sizeof(nome),stdin);
    nome[strcspn(nome,"\n")]='\0';
    int indice=1;

    strcat(nomeDAT,nome);
    strcat(nomeDAT,extensao);

    // Verifica se o arquivo existe
    while (access(nomeDAT, F_OK) != -1) {
        scanf(" %c", &resposta);

        if (resposta == 's' || resposta == 'S') {
            break;
        } else if (resposta == 'n' || resposta == 'N') {
            snprintf(nomeDAT, sizeof(nomeDAT), "%s_%d%s", nome, indice, extensao);
            indice++;
        }
    }

    arquivo = fopen(nomeDAT,"w");

    if (arquivo == NULL) {
        return;
    }

    for(int i=0;i<256;i++){
        fprintf(arquivo,"%d\n",memDados[i]);
    }

    fclose(arquivo);
}

void contabilizaEstat(instrucao *memoria, estatInstrucoes *estat, int pc){
    switch(memoria[pc].tipoInst){
        case 0:
            switch(memoria[pc].opcode){
                case 4:
                    (*estat).addi++;
                    break;
                case 8:
                    (*estat).beq++;
                    break;
                case 11:
                    (*estat).lw++;
                    break;
                case 15:
                    (*estat).sw++;
                    break;
            }
            (*estat).tipoI++;
            break;
        case 1:
            (*estat).j++;
            (*estat).tipoJ++;
            break;
        case 2:
        switch(memoria[pc].funct){
            case 0:
                    (*estat).add++;
                    break;
                case 2:
                    (*estat).sub++;
                    break;
                case 4:
                    (*estat).and++;
                    break;
                case 5:
                    (*estat).or++;
                    break;
            }
            (*estat).tipoR++;
            break;
    }

}

void salvaEstado(historico *hist, int pc, int *memDados, int *bReg, estatInstrucoes *estatInst, registradoresPipeline *pipe) {
    Node *novoNode = (Node *)malloc(sizeof(Node));
    if(novoNode == NULL) return;

    novoNode->st.pc = pc;
    novoNode->st.estat = *estatInst;
    novoNode->st.pipe = *pipe;
    memcpy(novoNode->st.memDados, memDados, 256 * sizeof(int));
    memcpy(novoNode->st.bReg, bReg, 8 * sizeof(int));

    novoNode->next = hist->topo;
    hist->topo = novoNode;
}

void voltaInstrucao(historico *hist, int *pc, int *memDados, int *bReg, estatInstrucoes *estatInst, registradoresPipeline *pipe) {
    if(hist->topo == NULL) return; // Nenhuma instrução para voltar

    Node *temp = hist->topo;
    hist->topo = temp->next;

    *pc = temp->st.pc;
    *estatInst = temp->st.estat;
    *pipe = temp->st.pipe;
    memcpy(memDados, temp->st.memDados, 256 * sizeof(int));
    memcpy(bReg, temp->st.bReg, 8 * sizeof(int));

    free(temp);
}


void liberaHistorico(historico *hist) {
    while(hist->topo != NULL) {
        Node *temp = hist->topo;
        hist->topo = hist->topo->next;
        free(temp);
    }
}

char *imprimeInstrucao(instrucao *memoria, int pc) {

    static char buffer[64];

    if(memoria[pc].decodificado == 0){
        decodificaInst(&memoria[pc]);
    }

    switch(memoria[pc].opcode){

        case 0:
            switch(memoria[pc].funct){
                case 0:
                    sprintf(buffer, "add $%d, $%d, $%d",
                            memoria[pc].rd,
                            memoria[pc].rs,
                            memoria[pc].rt);
                    break;

                case 2:
                    sprintf(buffer, "sub $%d, $%d, $%d",
                            memoria[pc].rd,
                            memoria[pc].rs,
                            memoria[pc].rt);
                    break;

                case 4:
                    sprintf(buffer, "and $%d, $%d, $%d",
                            memoria[pc].rd,
                            memoria[pc].rs,
                            memoria[pc].rt);
                    break;

                case 5:
                    sprintf(buffer, "or $%d, $%d, $%d",
                            memoria[pc].rd,
                            memoria[pc].rs,
                            memoria[pc].rt);
                    break;

                default:
                    sprintf(buffer, "R-type");
            }
            break;

        case 2:
            sprintf(buffer, "j %d", memoria[pc].addr);
            break;

        case 4:
            sprintf(buffer, "addi $%d, $%d, %d",
                    memoria[pc].rt,
                    memoria[pc].rs,
                    memoria[pc].imm);
            break;

        case 8:
            sprintf(buffer, "beq $%d, $%d, %d",
                    memoria[pc].rs,
                    memoria[pc].rt,
                    memoria[pc].imm);
            break;

        case 11:
            sprintf(buffer, "lw $%d, %d($%d)",
                    memoria[pc].rt,
                    memoria[pc].imm,
                    memoria[pc].rs);
            break;

        case 15:
            sprintf(buffer, "sw $%d, %d($%d)",
                    memoria[pc].rt,
                    memoria[pc].imm,
                    memoria[pc].rs);
            break;

        default:
            sprintf(buffer, "NOP");
            break;
    }

    return buffer;
}

// No arquivo hazards.c (ou adicione o protótipo no hazards.h)
void forwardingUnit(ID_EX *ID_EX, EX_MEM *EX_MEM, MEM_WB *MEM_WB, uint8_t *forwardA, uint8_t *forwardB) {
    
    *forwardA = 0;
    *forwardB = 0;
    
    if (EX_MEM->sinais.EscReg && (EX_MEM->rd != 0) && (EX_MEM->rd == ID_EX->rs)) {
        *forwardA = 2;
    }
    else if (MEM_WB->sinais.EscReg && (MEM_WB->rd != 0) && (MEM_WB->rd == ID_EX->rs)) {
        *forwardA = 1;
    }
    
    if (EX_MEM->sinais.EscReg && (EX_MEM->rd != 0) && (EX_MEM->rd == ID_EX->rt)) {
        *forwardB = 2;
    }
    else if (MEM_WB->sinais.EscReg && (MEM_WB->rd != 0) && (MEM_WB->rd == ID_EX->rt)) {
        *forwardB = 1;
    }
}

char *nomeInstrucao(uint8_t opcode, uint8_t funct){

    if(opcode==0){

        switch(funct){

            case 0:return "ADD";
            case 2:return "SUB";
            case 4:return "AND";
            case 5:return "OR";

            default:return "R";
        }
    }

    switch(opcode){

        case 2:return "J";

        case 4:return "ADDI";

        case 8:return "BEQ";

        case 11:return "LW";

        case 15:return "SW";

        default:return "NOP";
    }

}

char *nomeULA(uint8_t op){

    switch(op){

        case 0:
            return "ADD";

        case 1:
            return "SUB";

        case 2:
            return "AND";

        case 3:
            return "OR";

        default:
            return "--";
    }

}

void decodifica(instrucao *instrucao){

    instrucao->opcode = instrucao->instrucao >> 12; // Pega os 4 bits do opcode

    switch(instrucao->opcode){
        case 0:
            instrucao->tipoInst = tipoR;
            instrucao->rs = (instrucao->instrucao >> 9) & 0x7; // pega os 3 bits do rs (desloca 6 bits para a direita e pega os 3 mais significativos que ficaram)
            instrucao->rt = (instrucao->instrucao >> 6) & 0x7; // pega os 3 bits do rt
            instrucao->rd = (instrucao->instrucao >> 3) & 0x7; // pega os 3 bits do rd
            instrucao->funct = (instrucao->instrucao) & 0x7;
            break;

        case 2:
            instrucao->tipoInst = tipoJ;
            instrucao->addr = (instrucao->instrucao) &0xFF; // pega os 8 bits do adress
            break;

        default:
            instrucao->tipoInst = tipoI;
            instrucao->rs = (instrucao->instrucao >> 9) &0x7; // pega os 3 bits do rs
            instrucao->rt = (instrucao->instrucao >> 6) &0x7; // pega os 3 bits do rt
            instrucao->imm = (instrucao->instrucao) &0x3F; // pega os 6 bits do imediato (deve passar por um extensor antes da ULA)
            instrucao->imm = extensorBit(instrucao->imm);
    }
}

// TODO: Verificar se está sendo usado
int Verifica_Bolha(sinaisUC sinais){
    return !(sinais.EscReg ||
             sinais.EscMem ||
             sinais.branch ||
             sinais.jump);
}

void printBorda(int linhaspainel, int colunaspainel){

    attron(A_BOLD | COLOR_PAIR(4));

    // Linhas horizontais
    mvhline(0, 1, ACS_HLINE, colunaspainel - 2);
    mvhline(linhaspainel - 1, 1, ACS_HLINE, colunaspainel - 2);

    // Linhas verticais
    mvvline(1, 0, ACS_VLINE, linhaspainel - 2);
    mvvline(1, colunaspainel - 1, ACS_VLINE, linhaspainel - 2);

    // Cantos
    mvaddch(0, 0, ACS_ULCORNER);
    mvaddch(0, colunaspainel - 1, ACS_URCORNER);
    mvaddch(linhaspainel - 1, 0, ACS_LLCORNER);
    mvaddch(linhaspainel - 1, colunaspainel - 1, ACS_LRCORNER);

    attroff(A_BOLD | COLOR_PAIR(4));
}