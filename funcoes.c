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

void run_pipeline(instrucao *memoria, int *bReg, int *pc, int *memDados, registradoresPipeline *pipe, estatInstrucoes *estatInst)
{
    int acabou = 0;

    while(!acabou)
    {
        step_pipeline(memoria, bReg, pc, memDados, pipe, estatInst);
        
        // TODO: É necessário arrumar a condição de finalização do programa
        // Atualmente fica em loop pois Verifica_Bolha considera escReg = 1 como uma instrução útil
        if(pipe->regIF_ID_atual.inst.instrucao == 0 && Verifica_Bolha(pipe->regID_EX_atual.sinais) && Verifica_Bolha(pipe->regEX_MEM_atual.sinais) && Verifica_Bolha(pipe->regMEM_WB_atual.sinais) && (*pc >= 256 || memoria[*pc].instrucao == 0) && (*pc >= 256 ||memoria[*pc].instrucao == 0)){  
            acabou = 1;
        }
    }
}

void step_pipeline(instrucao *memoria, int *bReg, int *pc, int *memDados, registradoresPipeline *pipe, estatInstrucoes *estatInst) {    
    
    // [CORREÇÃO EFETIVA]: Uma instrução só conta nas estatísticas se:
    // 1. O opcode for diferente de 0 (Instruções Tipo-I e Tipo-J)
    // 2. OU se o opcode for 0 (Tipo-R), mas o registrador destino 'rd' for diferente de 0 (evita contar NOPs/Bolhas)
    int instrucao_valida = (pipe->regMEM_WB_atual.opcode != 0) || 
                           (pipe->regMEM_WB_atual.opcode == 0 && pipe->regMEM_WB_atual.rd != 0);

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
    Executa_EX(&pipe->regID_EX_atual, &pipe->regEX_MEM_novo);
    Executa_ID(&pipe->regID_EX_novo, &pipe->regIF_ID_atual, bReg);

    tipoHazard hazard = unidadeDetecHazards(&pipe->regIF_ID_atual, &pipe->regID_EX_atual, &pipe->regEX_MEM_atual);
    
    if(hazard == hazardDados) { 
        insereStall(&pipe->regID_EX_novo.sinais);
        pipe->regIF_ID_novo = pipe->regIF_ID_atual;
        estatInst->stalls++;
        (*pc)--;
    }
    else if(hazard == hazardControle) { 
        if (pipe->regID_EX_atual.sinais.jump == 1) {
            *pc = pipe->regID_EX_atual.imm;
        } else if (pipe->regEX_MEM_atual.sinais.branch == 1) {
            *pc = pipe->regEX_MEM_atual.ulaSaida;
        }
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

void atualiza_regs_pipeline(registradoresPipeline *pipe) {
    pipe->regIF_ID_atual = pipe->regIF_ID_novo;
    pipe->regID_EX_atual = pipe->regID_EX_novo;
    pipe->regEX_MEM_atual = pipe->regEX_MEM_novo;
    pipe->regMEM_WB_atual = pipe->regMEM_WB_novo;
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
    
    if(*pc >= 256 || memoria[*pc].instrucao == 0) { // verifica se ainda tem inst
        IF_ID->inst.instrucao = 0; // chegou no fim insere inst vazia 
        return;
    }
    IF_ID->pc = *pc;
    IF_ID->inst = memoria[*pc]; // guarda a instrução
   (*pc)++;

}

void Executa_ID(ID_EX *ID_EX, IF_ID *IF_ID, int *bReg) {

    // decodifica normal
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

void Executa_EX(ID_EX *ID_EX, EX_MEM *EX_MEM) {
    if (ID_EX->opcode == 0 && ID_EX->rd == 0 && ID_EX->rs == 0 && ID_EX->rt == 0) {
        EX_MEM->opcode = 0;
        EX_MEM->rd = 0;
        EX_MEM->ulaSaida = 0;
        EX_MEM->sinais.EscReg = 0;
        EX_MEM->sinais.branch = 0;
        EX_MEM->sinais.jump = 0;
        return;
    }

    int op2;
    int zero;
    int overflow;

    if(ID_EX->sinais.UlaFonte)
        op2 = ID_EX->imm;
    else
        op2 = ID_EX->B;

    EX_MEM->ulaSaida = ULA(ID_EX->A, op2, ID_EX->sinais.ulaOp, &zero, &overflow);

    EX_MEM->opcode = ID_EX->opcode;
    EX_MEM->B = ID_EX->B;
    EX_MEM->sinais = ID_EX->sinais;

    if(ID_EX->sinais.RegDst)
        EX_MEM->rd = ID_EX->rd;
    else
        EX_MEM->rd = ID_EX->rt;

}

void Executa_MEM(EX_MEM *EX_MEM, MEM_WB *MEM_WB,int *memDados){
    if (EX_MEM->opcode == 0 && EX_MEM->rd == 0) {
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

void insereStall(sinaisUC *sinais){
    sinais->EscMem = 0;
    sinais->EscReg = 0;
    sinais->IncPC = 0;
    sinais->branch = 0;
    sinais->MemParaReg = 0;
    sinais->RegDst = 0;
    sinais->UlaFonte = 0;
    sinais->ulaOp = 0;
    sinais->jump = 0;
}

void insereFlush(registradoresPipeline *pipe){
    insereStall(&pipe->regID_EX_novo.sinais);
    pipe->regIF_ID_novo.inst.instrucao = 0;
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

/*
void imprimeEstatistica(estatInstrucoes estatInst){
    estatInst.CPI = 0;
    if(estatInst.total > 0){
        estatInst.CPI = (float)estatInst.ciclos / (float)estatInst.total;
    }

    //printf("\n========================================\n");
    //printf("      Estatísticas do Simulador:\n");
    //printf("========================================\n");

    //printf("\nTotal executadas: %d\n", estatInst.total);
    //printf("Ciclos: %d\n", estatInst.ciclos);

    //printf("CPI (Ciclos por Instrução): %.2f\n", estatInst.CPI);
    //printf("Stalls: %d\n", estatInst.stalls);

    //printf("\nEstatísticas de instruções\n");
    //printf("\nPor tipo:\n");
    //printf("Tipo R: %d\n", estatInst.tipoR);
    //printf("Tipo I: %d\n", estatInst.tipoI);
    //printf("Tipo J: %d\n", estatInst.tipoJ);

    //printf("\nDetalhamento por instrução:\n");
    //printf("R -> add: %d | sub: %d | and: %d | or: %d\n",
           estatInst.add, estatInst.sub, estatInst.and, estatInst.or);
    //printf("I -> addi: %d | beq: %d | lw: %d | sw: %d\n",
           estatInst.addi, estatInst.beq, estatInst.lw, estatInst.sw);
    //printf("J -> j: %d\n", estatInst.j);


    //printf("========================================\n\n");

    NCurse
    clear();
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(1, 5, "+------------------------------------------------+");
    mvprintw(2, 5, "|           ESTATISTICAS DO PIPELINE             |");
    mvprintw(3, 5, "+------------------------------------------------+");
    attroff(COLOR_PAIR(1) | A_BOLD);

    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(5, 5, "[ Métricas Globais ]");
    attroff(COLOR_PAIR(2) | A_BOLD);

    mvprintw(7, 5, "Total executadas: %4d", estatInst.total);
    mvprintw(8, 5, "Ciclos: %4d", estatInst.ciclos);
    mvprintw(9, 5, "CPI (Ciclos por Instrucao): %4.2f", estatInst.CPI);
    mvprintw(10, 5, "Stalls: %4d", estatInst.stalls);

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
    mvprintw(23, 5, "|                [ Voltar ]                      |");
    mvprintw(24, 5, "+------------------------------------------------+");
    attroff(COLOR_PAIR(1) | A_BOLD);

    refresh();
    
    getch();
}
*/


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

void imprimeInstrucao(instrucao *memoria, int pc) {
    if((memoria)[pc].decodificado==0){
        decodificaInst(&memoria[pc]);
    }

    switch(memoria[pc].opcode){
        case 0: // opcode = 0000

            if(memoria[pc].funct==0){
                //mvprintfw("add $%d, $%d, $%d", (memoria)[pc].rd, (memoria)[pc].rs, (memoria)[pc].rt);
            }
            else if((memoria)[pc].funct==2){
                //printf("sub $%d, $%d, $%d", (memoria)[pc].rd, (memoria)[pc].rs, (memoria)[pc].rt);
            }
            else if((memoria)[pc].funct==4){
                //printf("and $%d, $%d, $%d", (memoria)[pc].rd, (memoria)[pc].rs, (memoria)[pc].rt);
            }
            else if((memoria)[pc].funct==5){
                //printf("or $%d, $%d, $%d", (memoria)[pc].rd, (memoria)[pc].rs, (memoria)[pc].rt);
            }
            break;

        case 2: // opcode = 0010 - J
            //printf("j %d%*s", (memoria)[pc].addr, x, "");

            break;

        case 4: // opcode = 0100 - Addi
            //printf("addi $%d, $%d, %d", (memoria)[pc].rt, (memoria)[pc].rs, (memoria)[pc].imm);

            break;

        case 8: // opcode = 1000 - BEQ
            //printf("beq $%d, $%d, %d", (memoria)[pc].rs, (memoria)[pc].rt, (memoria)[pc].imm);

            break;

        case 11: // opcode = 1011 - lw
            //printf("lw $%d, %d($%d)", (memoria)[pc].rt, (memoria)[pc].imm, (memoria)[pc].rs);

            break;

        case 15: // opcode = 1111 - sw
            //printf("sw $%d, %d($%d)", (memoria)[pc].rt, (memoria)[pc].imm, (memoria)[pc].rs);

            break;
    }
}

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
        attroff(A_BOLD | COLOR_PAIR(1));

        for(int i = 1; i < colunaspainel - 1; i++){
                mvprintw(linhaspainel/3, i, "_");
                mvprintw(linhaspainel/1.5, i, "_");
                mvprintw(linhaspainel - 4, i, "_");
        }

        for(int j = 1; j < linhaspainel - 1; j++){
            mvprintw(j/3 + 1, colunaspainel/5, "|");
            mvprintw(j/3 + 1, (colunaspainel/5) * 2, "|");
            mvprintw(j/3 + 1, (colunaspainel/5) * 3, "|");
            mvprintw(j - linhaspainel/3 + 2, (colunaspainel/5) * 4, "|");

        }

        attron(A_BOLD | COLOR_PAIR(3));
        mvprintw(1, colunaspainel/5 - 19, "IF");
        mvprintw(1, (colunaspainel/5 * 2) - 19, "ID");
        mvprintw(1, (colunaspainel/5 * 3) - 19, "EX");
        mvprintw(1, (colunaspainel/5 * 4) - 19, "MEM");
        mvprintw(1, (colunaspainel/5 * 5) - 19, "WB");
        attroff(A_BOLD | COLOR_PAIR(3));
        
        //  -> PC: %d | Inst: %04X (%s)", pipe->regIF_ID_atual.pc, pipe->regIF_ID_atual.inst.instrucao, pipe->regIF_ID_atual.inst.mem);
        //mvprintw(5, 3, "ID/EX  -> A: %d | B: %02d | Imm: %d | Opcode: %X | rs: %d rt: %d rd: %d", pipe->regID_EX_atual.A, pipe->regID_EX_atual.B, pipe->regID_EX_atual.imm, pipe->regID_EX_atual.opcode, pipe->regID_EX_atual.rs, pipe->regID_EX_atual.rt, pipe->regID_EX_atual.rd);
        //mvprintw(6, 3, "EX/MEM -> UlaSaida: %d | B: %d | rd: %d", pipe->regEX_MEM_atual.ulaSaida, pipe->regEX_MEM_atual.B, pipe->regEX_MEM_atual.rd);
        //mvprintw(7, 3, "MEM/WB -> MemDado: %d | UlaSaida: %d | rd: %d", pipe->regMEM_WB_atual.mem, pipe->regMEM_WB_atual.ulaSaida, pipe->regMEM_WB_atual.rd);
        //mvprintw(8, 3, "PC Atual: %d", pc);

        attron(A_BOLD | COLOR_PAIR(2));
        mvprintw(linhaspainel/2 - 5, colunaspainel - 30, "BANCO DE REGISTRADORES");
        attroff(A_BOLD | COLOR_PAIR(2));
        
        for(int i = 0; i < 8; i++) {
            mvprintw((linhaspainel/2) - 2 + i, colunaspainel - 23, "$%d: %d", i, bReg[i]);
        }

        attron(A_BOLD | COLOR_PAIR(3));
        mvprintw(linhaspainel/2 - 5, 20, "ESTATÍSTICAS");
        attroff(A_BOLD | COLOR_PAIR(3));

        if(estatInst->ciclos > 0){
            estatInst->CPI = (float)estatInst->ciclos / (estatInst->total > 0 ? estatInst->total : 1);
        } 
        
        mvprintw(linhaspainel/2 - 3, 3, "Ciclos: %d  |  Stalls: %d  |  CPI: %.2f", estatInst->ciclos, estatInst->stalls, estatInst->CPI);
        mvprintw(linhaspainel/2 - 2, 3, "R-Type: %d (add: %d, sub: %d, and: %d, or: %d)", estatInst->tipoR, estatInst->add, estatInst->sub, estatInst->and, estatInst->or);
        mvprintw(linhaspainel/2 - 1, 3, "I-Type: %d (addi: %d, beq: %d, lw: %d, sw: %d) | J-Type: %d", estatInst->tipoI, estatInst->addi, estatInst->beq, estatInst->lw, estatInst->sw, estatInst->tipoJ);

        attron(A_BOLD | COLOR_PAIR(4));
        mvprintw(linhaspainel - 4, 3, " OPÇÕES DE EXECUÇÃO ");
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
                    run_pipeline(memoria, bReg, &pc, memDados, pipe, estatInst);
                } 
                else if(selecionado == 1) { // Step
                    salvaEstado(hist, pc, memDados, bReg, estatInst, pipe);
                    step_pipeline(memoria, bReg, &pc, memDados, pipe, estatInst);
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

int Verifica_Bolha(sinaisUC sinais) {
    if ((sinais.EscReg || sinais.EscMem || sinais.branch || sinais.jump)) {
        return 0; // Tem algum sinal ativo, então não é bolha
    }
    return 1; // Tudo zero, É uma bolha
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

    for(int i = 0; i < linhaspainel; i++) {
        mvprintw(i, 0, "|");
        mvprintw(i, colunaspainel - 1, "|");
    }

    for(int j = 0; j < colunaspainel; j++) {
        mvprintw(0, j, "=");
        mvprintw(linhaspainel - 1, j, "=");
    }

}