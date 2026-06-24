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

void run_pipeline(historico *hist, instrucao *memoria, int *bReg, int *pc, int *memDados, registradoresPipeline *pipe, estatInstrucoes *estatInst)
{
    while (1)
    {
        step_pipeline(hist, memoria, bReg, pc, memDados, pipe, estatInst);

        if ((*pc >= 256 || memoria[*pc].instrucao == 0) &&
            pipe->regIF_ID_atual.inst.instrucao == 0 &&
            pipe->regID_EX_atual.opcode == 0 &&
            pipe->regEX_MEM_atual.opcode == 0 &&
            pipe->regMEM_WB_atual.opcode == 0)
        {
            break;
        }
    }
}

void step_pipeline(historico *hist, instrucao *memoria, int *bReg, int *pc, int *memDados, registradoresPipeline *pipe, estatInstrucoes *estatInst) {    
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
        *pc = pipe->regID_EX_atual.imm;

    else if(pipe->regEX_MEM_atual.sinais.branch)
        *pc = pipe->regEX_MEM_atual.ulaSaida;

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
            sinais->MemParaReg = 1;
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
            sinais->MemParaReg = 1;
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
            sinais->MemParaReg = 0;
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
    
    //printf("\n\n==================== WB =======================\n");

    if (MEM_WB->rd == 0) {
        bReg[0] = 0;
        return;
    }
    
    if(MEM_WB->sinais.EscReg == 1) {
        int8_t dadoFinal;
        
        if(MEM_WB->sinais.MemParaReg == 1){
            dadoFinal = MEM_WB->mem;
            //printf("\n[ WB ] Dado %d preparado para escrita no banco de registradores. (Vindo da memória)\n", dadoFinal);
        }else{
            dadoFinal = MEM_WB->ulaSaida;
            //printf("\n[ WB ] Dado %d preparado para escrita no banco de registradores. (Vindo da ULA)\n", dadoFinal);
        }

        escreveRegistrador(bReg, MEM_WB->rd, dadoFinal, MEM_WB->sinais.EscReg);
        //pri("\n[ WB ] %d escrito no registrador $%d.\n", dadoFinal, MEM_WB->rd);
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

                case 10:          // Enter no Linux/Unix
                case KEY_ENTER:   // Enter do teclado numérico
                    confirmou = 1; // Usuário tomou uma decisão
                    break;
            }
        }

        // Processa a decisão do usuário após ele apertar ENTER
        if(selecionado == 0){
            /* Sobrescrever: sai do loop "while(access...)" e vai direto para a criação */
            break; 
        } else {
            /* Não sobrescrever: gera um novo nome e o "while" testará se ele existe */
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

    //printf("\nDigite o nome do arquivo que deseja salvar (.dat): ");
    fgets(nome, sizeof(nome),stdin);
    nome[strcspn(nome,"\n")]='\0';
    int indice=1;

    strcat(nomeDAT,nome);
    strcat(nomeDAT,extensao);

    // Verifica se o arquivo existe
    while (access(nomeDAT, F_OK) != -1) {
        //printf("\nJá existe um arquivo com o nome %s, deseja sobrescrever? (s/n): ", nomeDAT);
        scanf(" %c", &resposta);

        if (resposta == 's' || resposta == 'S') {
            break;
        } else if (resposta == 'n' || resposta == 'N') {
            snprintf(nomeDAT, sizeof(nomeDAT), "%s_%d%s", nome, indice, extensao);
            indice++;
        } else {
            //printf("\nOpção inválida. Tente novamente.\n");
        }
    }

    arquivo = fopen(nomeDAT,"w");

    if (arquivo == NULL) {
        //printf("\nErro ao criar arquivo\n");
        return;
    }

    for(int i=0;i<256;i++){
        fprintf(arquivo,"%d\n",memDados[i]);
    }

    fclose(arquivo);

    //printf("\nArquivo '%s' salvo!\n",nomeDAT);
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

//IMPRIMIR TODO SIMULADOR
void imprimeTodoSimulador(int colunaspainel, int linhaspainel, registradoresPipeline *pipe, estatInstrucoes *estatInst, int *bReg, int pc, instrucao *memoria, int *memDados, historico *hist) {
    int selecionado = 0;
    int tecla;
    char *opcoesMenu[] = { "Run Pipeline", "Step Pipeline", "Step Back", "Voltar Menu" };
    int totalOpcoes = 4;

    while(1) {
        clear();
        printBorda(linhaspainel, colunaspainel);

        attron(A_BOLD | COLOR_PAIR(1));
        mvprintw(0, colunaspainel/2 - 11, " SIMULADOR DE PIPELINE ");

        mvhline(1, 2, ACS_HLINE, colunaspainel - 4);// primeira linha
        
        mvhline(linhaspainel / 3, 2, ACS_HLINE, colunaspainel - 4);// segunda linha
        mvhline(linhaspainel / 3 + 1, 2, ACS_HLINE, colunaspainel - 4);

        mvhline((2 * linhaspainel)/3 + 1, 2, ACS_HLINE, colunaspainel - 4);//terceira linha
        mvhline((linhaspainel/3) * 2, 2, ACS_HLINE, colunaspainel - 4);
        
        mvhline(linhaspainel - 4, 2, ACS_HLINE, (colunaspainel - (colunaspainel/5)) - 4);// quarta linha
        mvhline(linhaspainel - 3, 2, ACS_HLINE, (colunaspainel - (colunaspainel/5)) - 4);
        

        
        mvvline(1, 2, ACS_VLINE, linhaspainel - 2);//pimeira coluna
        mvvline(1, colunaspainel / 5 - 1, ACS_VLINE, linhaspainel/3 - 1);
        
        mvvline(1, colunaspainel / 5, ACS_VLINE, linhaspainel/3);//segunda coluna
        mvvline(1, (colunaspainel / 5)*2, ACS_VLINE, linhaspainel/3);
        mvvline(1, (colunaspainel / 5)*2 - 1, ACS_VLINE, linhaspainel/3);
        
        mvvline(1, (colunaspainel / 5)*3, ACS_VLINE, linhaspainel/3);//terceira coluna
        mvvline(1, (colunaspainel / 5)*3 - 1, ACS_VLINE, linhaspainel/3);

        mvvline(1, (colunaspainel / 5)*4, ACS_VLINE, linhaspainel/3);//quarta coluna 
        mvvline(1, (colunaspainel / 5)*4 - 1, ACS_VLINE, linhaspainel/3);

        mvvline(1, colunaspainel - 3, ACS_VLINE, linhaspainel - 2);//quinta  coluna
        
        mvaddch(linhaspainel/3, (colunaspainel/5)*2, ACS_BTEE);
        mvaddch(linhaspainel/3, (colunaspainel/5)*3, ACS_BTEE);
        mvaddch(linhaspainel/3, (colunaspainel/5)*4, ACS_PLUS);

        
        //cantos IF
        mvaddch(1, 2, ACS_ULCORNER);
        mvaddch(linhaspainel/3, 2, ACS_LLCORNER);
        mvaddch(linhaspainel/3, colunaspainel/5, ACS_LLCORNER);
        mvaddch(1, colunaspainel/5 - 1, ACS_URCORNER);
        
        //cantos ID
        mvaddch(1, colunaspainel/5,ACS_ULCORNER);
        mvaddch(linhaspainel/3, colunaspainel/5 - 1, ACS_LRCORNER);
        mvaddch(linhaspainel/3, (colunaspainel/5) * 2 - 1, ACS_LRCORNER);
        mvaddch(1, ((colunaspainel/5) * 2) - 1, ACS_URCORNER);

        //cantos EX
        mvaddch(1, (colunaspainel/5) * 2,ACS_ULCORNER);
        mvaddch(linhaspainel/3, (colunaspainel/5) * 2, ACS_LLCORNER);
        mvaddch(1, (colunaspainel/5) * 3 - 1, ACS_URCORNER);
        mvaddch(linhaspainel/3, (colunaspainel/5) * 3 - 1, ACS_LRCORNER);

        //cantos MEM
        mvaddch(1, (colunaspainel/5) * 3,ACS_ULCORNER);
        mvaddch(linhaspainel/3, (colunaspainel/5) * 3, ACS_LLCORNER);
        mvaddch(1, (colunaspainel/5) * 4 - 1, ACS_URCORNER);
        mvaddch(linhaspainel/3, (colunaspainel/5) * 4 - 1, ACS_LRCORNER);

        //cantos WB
        mvaddch(1, (colunaspainel/5) * 4,ACS_ULCORNER);
        mvaddch(linhaspainel/3, (colunaspainel/5) * 4, ACS_LLCORNER);
        mvaddch(1, colunaspainel - 3, ACS_URCORNER);
        mvaddch(linhaspainel/3, colunaspainel - 3, ACS_LRCORNER);

        //cantos estatisticas
        mvaddch(linhaspainel / 3 + 1, 2,ACS_ULCORNER);
        mvaddch(linhaspainel / 3 + 1, colunaspainel - 3, ACS_URCORNER);
        mvaddch((linhaspainel/3) * 2, 2, ACS_LLCORNER);
        mvaddch(((linhaspainel/3) * 2), colunaspainel - 3, ACS_LRCORNER);

        //cantos menu
        mvaddch((linhaspainel / 3) * 2 + 1, 2, ACS_ULCORNER);
        mvaddch((linhaspainel / 3) * 2 + 1, colunaspainel - 3, ACS_URCORNER);
        mvaddch(linhaspainel - 4, 2, ACS_LLCORNER);
        mvaddch(linhaspainel - 3, 2, ACS_ULCORNER);

        attroff(A_BOLD | COLOR_PAIR(1));

        attron(A_BOLD | COLOR_PAIR(2));
        mvprintw(1, (colunaspainel/5) - colunaspainel/10 - 2, "  IF  ");
        mvprintw(3, (colunaspainel/5) / 2 - 5,"PC : %d",pipe->regIF_ID_atual.pc);
        mvprintw(4, (colunaspainel/5 - 25),"HEX: %04X",pipe->regIF_ID_atual.inst.instrucao);
        mvprintw(6, (colunaspainel/5 - 25),"%s",pipe->regIF_ID_atual.inst.mem);
        attroff(A_BOLD | COLOR_PAIR(2));
        
        attron(A_BOLD | COLOR_PAIR(3));
        mvprintw(1, (colunaspainel/5 * 2) - colunaspainel/10 - 3, "  ID  ");
        mvprintw(3, (colunaspainel/5 * 2) - 21,"Instr : %s",
        nomeInstrucao(pipe->regID_EX_atual.opcode,pipe->regID_EX_atual.funct));
        mvprintw(12, (colunaspainel/5 * 2) - 21,"ASM: %s",imprimeInstrucao(memoria, pc - 2));
        mvprintw(4, (colunaspainel/5 * 2) - 21,"rs : %d",pipe->regID_EX_atual.rs);
        mvprintw(5, (colunaspainel/5 * 2) - 21,"rt : %d",pipe->regID_EX_atual.rt);
        mvprintw(6, (colunaspainel/5 * 2) - 21,"rd : %d",pipe->regID_EX_atual.rd);
        mvprintw(7, (colunaspainel/5 * 2) - 21,"A  : %d",pipe->regID_EX_atual.A);
        mvprintw(8, (colunaspainel/5 * 2) - 21,"B  : %d",pipe->regID_EX_atual.B);
        mvprintw(9, (colunaspainel/5 * 2) - 21,"Imm: %d",pipe->regID_EX_atual.imm);
        mvprintw(10, (colunaspainel/5 * 2) - 21,"Fun: %d",pipe->regID_EX_atual.funct);
        attroff(A_BOLD | COLOR_PAIR(3));

        attron(A_BOLD | COLOR_PAIR(4));
        mvprintw(1, (colunaspainel/5 * 3) - colunaspainel/10 - 3, "  EX  ");
        mvprintw(3, (colunaspainel/5 * 3) - 22,"ULA : %d",pipe->regEX_MEM_atual.ulaSaida);
        mvprintw(4, (colunaspainel/5 * 3) - 22,"B   : %d",pipe->regEX_MEM_atual.B);
        mvprintw(5, (colunaspainel/5 * 3) - 22,"RD  : %d",pipe->regEX_MEM_atual.rd);
        mvprintw(6, (colunaspainel/5 * 3) - 22,"ALU : %s",nomeULA(pipe->regEX_MEM_atual.sinais.ulaOp));
        mvprintw(7, (colunaspainel/5 * 3) - 22,"Branch : %d",pipe->regEX_MEM_atual.sinais.branch);
        mvprintw(8, (colunaspainel/5 * 3) - 22,"Jump   : %d",pipe->regEX_MEM_atual.sinais.jump);
        attroff(A_BOLD | COLOR_PAIR(4));

        attron(A_BOLD | COLOR_PAIR(5));
        mvprintw(1, (colunaspainel/5 * 4) - colunaspainel/10 - 4, "  MEM  ");
        mvprintw(3, (colunaspainel/5 * 4) - 22,"ULA : %d",pipe->regMEM_WB_atual.ulaSaida);
        mvprintw(4, (colunaspainel/5 * 4) - 22,"MEM : %d",pipe->regMEM_WB_atual.mem);
        mvprintw(5, (colunaspainel/5 * 4) - 22,"RD  : %d",pipe->regMEM_WB_atual.rd);
        mvprintw(6, (colunaspainel/5 * 4) - 22,"EscMem : %d", pipe->regMEM_WB_atual.sinais.EscMem);
        mvprintw(7, (colunaspainel/5 * 4) - 22,"MemReg : %d", pipe->regMEM_WB_atual.sinais.MemParaReg);
        attroff(A_BOLD | COLOR_PAIR(5));

        attron(A_BOLD | COLOR_PAIR(6));
        mvprintw(1, (colunaspainel/5 * 5) - colunaspainel/10 - 4, "  WB  ");
        mvprintw(3, (colunaspainel/5 * 5) - 22,"Destino : $%d", pipe->regMEM_WB_novo.rd);
        mvprintw(4, (colunaspainel/5 * 5) - 22,"Valor   : %d", pipe->regMEM_WB_novo.sinais.MemParaReg ? pipe->regMEM_WB_novo.mem : pipe->regMEM_WB_novo.ulaSaida);
        mvprintw(5, (colunaspainel/5 * 5) - 22,"EscReg : %d", pipe->regMEM_WB_novo.sinais.EscReg);
        attroff(A_BOLD | COLOR_PAIR(6));

        //registro instrucoes
        attron(A_BOLD | COLOR_PAIR(3));
        mvprintw(linhaspainel/3 + 2, 4, "REGISTRO DE INSTRUÇÕES:");
        attroff(A_BOLD | COLOR_PAIR(3));

        Node *linhasLog[8];
        int qtdEncontrada = 0;
        
        Node *atualNode = hist->topo;
        
        while (atualNode != NULL && qtdEncontrada < 8) {
            linhasLog[qtdEncontrada] = atualNode;
            qtdEncontrada++;
            atualNode = atualNode->next;
        }

        int linhaVisualInicial = linhaspainel/3 + 4;
        int idxPrint = 0;

        for (int i = qtdEncontrada - 1; i >= 0; i--) {
            int pcSalvo = linhasLog[i]->st.pc;
            
            mvprintw(linhaVisualInicial + idxPrint, 4, "Ciclo %-2d |  Instrução: %-25s",  linhasLog[i]->st.estat.ciclos, imprimeInstrucao(memoria, pcSalvo));
            idxPrint++;
        }

        attron(A_BOLD | COLOR_PAIR(2));
        mvprintw(linhaspainel/2 - 5, colunaspainel - 30, "BANCO DE REGISTRADORES");
        
        for(int i = 0; i < 8; i++) {
            mvprintw((linhaspainel/2) - 2 + i, colunaspainel - 23, "$%d: %d", i, bReg[i]);
        }

        attroff(A_BOLD | COLOR_PAIR(2));

        attron(A_BOLD | COLOR_PAIR(3));
        mvprintw(linhaspainel/2 - 4, 61, "ESTATÍSTICAS");

        mvprintw(linhaspainel/2 - 2, 61,"Ciclos : %d",estatInst->ciclos);
        mvprintw(linhaspainel/2 - 1, 61,"Instr  : %d",estatInst->total);
        mvprintw(linhaspainel/2 - 0, 61,"Stalls : %d",estatInst->stalls);
        mvprintw(linhaspainel/2 + 1, 61,"CPI    : %.2f",estatInst->CPI);
        mvprintw(linhaspainel/2 + 2, 61,"R : %d",estatInst->tipoR);
        mvprintw(linhaspainel/2 + 3, 61,"I : %d",estatInst->tipoI);
        mvprintw(linhaspainel/2 + 4, 61,"J : %d",estatInst->tipoJ);
        mvprintw(linhaspainel/2 + 5, 61,"ADD:%d SUB:%d",estatInst->add,estatInst->sub);
        mvprintw(linhaspainel/2 + 6, 61,"LW:%d SW:%d",estatInst->lw,estatInst->sw);

        attroff(A_BOLD | COLOR_PAIR(3));

        attron(A_BOLD | COLOR_PAIR(4));
        mvprintw(linhaspainel - 3, 3, " OPÇÕES DE EXECUÇÃO ");
        attroff(A_BOLD | COLOR_PAIR(4));
        
        for(int i = 0; i < totalOpcoes; i++) {
            if(i == selecionado) {
                attron(A_REVERSE | A_BOLD);
            }

            mvprintw(linhaspainel - 2, 3 + (i * 18), "[ %s ]", opcoesMenu[i]);

            if(i == selecionado) {
                attroff(A_REVERSE | A_BOLD);
            }
        }

        refresh();
        tecla = getch();

        switch(tecla) {
            case KEY_LEFT:
                selecionado--;
                if(selecionado < 0) selecionado = totalOpcoes - 1;
                break;
            case KEY_RIGHT:
                selecionado++;
                if(selecionado >= totalOpcoes) selecionado = 0;
                break;
            case 10: // Enter
            
                if(selecionado == 0) { // Run
                    run_pipeline(hist, memoria, bReg, &pc, memDados, pipe, estatInst);
                }
                else if(selecionado == 1) { // Step
                    step_pipeline(hist, memoria, bReg, &pc, memDados, pipe, estatInst);
                } 
                else if(selecionado == 2) { // Back
                    voltaInstrucao(hist, &pc, memDados, bReg, estatInst, pipe);
                } 
                else if(selecionado == 3) { // Voltar
                    return;
                }
                break;
        }
    }
}


void desenhaQuadro(int y, int x, int h, int w, char *titulo){

    mvhline(y, x+1, ACS_HLINE, w-2);
    mvhline(y+h-1, x+1, ACS_HLINE, w-2);

    mvvline(y+1,x,ACS_VLINE,h-2);
    mvvline(y+1,x+w-1,ACS_VLINE,h-2);

    mvaddch(y,x,ACS_ULCORNER);
    mvaddch(y,x+w-1,ACS_URCORNER);
    mvaddch(y+h-1,x,ACS_LLCORNER);
    mvaddch(y+h-1,x+w-1,ACS_LRCORNER);

    attron(A_BOLD);

    mvprintw(y,x+2,"%s",titulo);

    attroff(A_BOLD);
}

// No arquivo hazards.c (ou adicione o protótipo no hazards.h)

void forwardingUnit(ID_EX *ID_EX, EX_MEM *EX_MEM, MEM_WB *MEM_WB, uint8_t *forwardA, uint8_t *forwardB) {
    *forwardA = 0; // 00 = Sem adiantamento
    *forwardB = 0;

    // --- ENCAMINHAMENTO PARA O OPERANDO A (rs) ---
    
    // Hazard EX: Maior prioridade (dado mais recente)
    if (EX_MEM->sinais.EscReg && (EX_MEM->rd != 0) && (EX_MEM->rd == ID_EX->rs)) {
        *forwardA = 2; // Equivalente ao binário 10
    }
    // Hazard MEM: Menor prioridade
    else if (MEM_WB->sinais.EscReg && (MEM_WB->rd != 0) && (MEM_WB->rd == ID_EX->rs)) {
        *forwardA = 1; // Equivalente ao binário 01
    }

    // --- ENCAMINHAMENTO PARA O OPERANDO B (rt) ---
    
    // Hazard EX: Maior prioridade
    if (EX_MEM->sinais.EscReg && (EX_MEM->rd != 0) && (EX_MEM->rd == ID_EX->rt)) {
        *forwardB = 2; // Equivalente ao binário 10
    }
    // Hazard MEM: Menor prioridade
    else if (MEM_WB->sinais.EscReg && (MEM_WB->rd != 0) && (MEM_WB->rd == ID_EX->rt)) {
        *forwardB = 1; // Equivalente ao binário 01
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

        case 0:return "ADD";

        case 1:return "SUB";

        case 2:return "AND";

        case 3:return "OR";

        default:return "--";
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

int Verifica_Bolha(sinaisUC sinais){
    return !(sinais.EscReg ||
             sinais.EscMem ||
             sinais.branch ||
             sinais.jump);
}

// TODO: Ajustar essas impressões - ncurses
void print_pipeline_state(registradoresPipeline *pipe, int ciclo) {
    (void)ciclo;
    //printf("\n========== Ciclo %d ==========\n", ciclo);

    int ocupados = 0;

    // IF
    if(pipe->regIF_ID_atual.inst.instrucao!=0) {
        //printf("IF : PC=%d opcode=%d\n", pipe->regIF_ID_atual.pc, pipe->regIF_ID_atual.inst.opcode);
        ocupados++;
    }
    else{
        //printf("Stall ou NOP\n");
    }

    // ID
    if(!Verifica_Bolha(pipe->regID_EX_atual.sinais)) {
        //printf("ID : opcode=%d rs=%d rt=%d rd=%d A=%d B=%d\n",pipe->regID_EX_atual.opcode, pipe->regID_EX_atual.rs, pipe->regID_EX_atual.rt,pipe->regID_EX_atual.rd, pipe->regID_EX_atual.A, pipe->regID_EX_atual.B);
        ocupados++;
    }
    else{
        //printf("Stall ou NOP\n");
    }

    // EX
    if(!Verifica_Bolha(pipe->regEX_MEM_atual.sinais)) {
        //printf("EX : opcode=%d ulaSaida=%d rd=%d\n",pipe->regEX_MEM_atual.opcode, pipe->regEX_MEM_atual.ulaSaida, pipe->regEX_MEM_atual.rd);
        ocupados++;
    }
    else{
        //printf("Stall ou NOP\n");
    }

    // MEM/WB - junta os dois pq WB só escreve o que veio de MEM
    if(!Verifica_Bolha(pipe->regMEM_WB_atual.sinais)) {
        //printf("MEM: opcode=%d ulaSaida=%d mem=%d rd=%d\n",pipe->regMEM_WB_atual.opcode, pipe->regMEM_WB_atual.ulaSaida,pipe->regMEM_WB_atual.mem, pipe->regMEM_WB_atual.rd);
        //printf("WB : opcode=%d vai escrever rd=%d\n",pipe->regMEM_WB_atual.opcode, pipe->regMEM_WB_atual.rd);
        ocupados++;
    }
    else{
        //printf("Stall ou NOP\n");
        //printf("Stall ou NOP\n");
    }

    //printf("Instruções no pipeline: %d/4\n", ocupados); // 4 instruções?
    //printf("============================\n\n");
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