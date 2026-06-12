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
        printf("\nAcesso negado!\n");
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
void lerMem(char *arq, instrucao **memoria, int linhas){
    *memoria = calloc(256, sizeof(instrucao));
    if(memoria == NULL){
    printf("\nMemoria não carregada!\n");
    return;
}
    arquivo = fopen(arq, "r");
    int i=0;
    char mem[17];

    if(arquivo==NULL){
        printf("\nPermissão negada!");
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

void imprimeMemorias(instrucao *memoria, int *memDados){
    int opt, x;
    do{
        printf("\n1. Memória de instruções\n2. Memória de dados\n");
        printf("\nSelecione uma das opções acima: ");
        scanf("%d", &opt);

        switch(opt){
            case 1:
	            x = 70;

                printf("\n%*sMemória de Instruções:\n\n", x, "");

                for (int linha = 0; linha < 64; linha++) {
                    printf(" %3d: %16s: ", linha, memoria[linha].mem);
                    imprimeInstrucao(memoria, linha);

                    printf("\t %3d: %16s: ", linha + 64, memoria[linha + 64].mem);
                    imprimeInstrucao(memoria, linha + 64);

                    printf("\t %3d: %16s: ", linha + 128, memoria[linha + 128].mem);
                    imprimeInstrucao(memoria, linha + 128);

                    printf("\t %3d: %16s: ", linha + 192, memoria[linha + 192].mem);
                    imprimeInstrucao(memoria, linha + 192);

                    printf("\n");
                }
                printf("\n");
                break;

            case 2:

                x = 20;

                printf("\n%*sMemória de Dados:\n\n", x, "");

                for (int linha = 0; linha < 64; linha++) {
                    printf("%3d: %3d\t %3d: %3d\t %3d: %3d\t %3d: %3d\n",
                    linha, memDados[linha],
                    linha + 64, memDados[linha + 64],
                    linha + 128, memDados[linha + 128],
                    linha + 192, memDados[linha + 192]);
                }
                printf("\n");
                break;

            default:
                printf("Opção inválida! Por favor, selecione uma das opções disponíveis.\n");
        }
    }while(opt<1 || opt>2);
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
        step_pipeline(memoria, bReg, pc,
            memDados, pipe, estatInst);
        
        // TODO: É necessário arrumar a condição de finalização do programa
        // Atualmente o loop encerra assim que o IF ê o HALT ou a memória acaba, 
        // porém é finalizar as instruções que já estavam sendo executadas antes
        if(*pc >= 256 ||memoria[*pc].instrucao == 0){  
            acabou = 1;
        }
    }
}

void step_pipeline(instrucao *memoria, int *bReg, int *pc, int *memDados, registradoresPipeline *pipe, estatInstrucoes *estatInst) {

    // Ordem invertida: WB -> MEM -> EX -> ID -> IF
    executaWB(&pipe->regMEM_WB_atual, bReg, estatInst);
    do_MEM(&pipe->regEX_MEM_atual, &pipe->regMEM_WB_novo, memDados);
    do_EX(&pipe->regID_EX_atual, &pipe->regEX_MEM_novo);
    do_ID(&pipe->regID_EX_novo, &pipe->regIF_ID_atual, bReg);

    int hazard = unidadeDetecHazards(&pipe->regIF_ID_atual, &pipe->regID_EX_atual, &pipe->regEX_MEM_atual);

    if(hazard==1){
        insereStall(&pipe->regID_EX_novo.sinais);
        pipe->regIF_ID_novo = pipe->regIF_ID_atual;
        estatInst->stalls++;
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

// PC
void programCounter(int *pc, sinaisUC *sinais, instrucao *instrucao, int zero){

    // JUMP
    if(sinais->jump == 1){
        *pc = instrucao->addr;
        printf("\nPC atual: %d.\n", *pc);
        return;
    }

    // BRANCH 
    if(sinais->branch == 1 && zero == 1){
        *pc = *pc + instrucao->imm + 1;
        printf("\nPC atual: %d.\n", *pc);
        return;
    }

    // execução normal
    (*pc)++;
    printf("\n[ PC+1 ]\nPC atual: %d.\n", *pc);
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
        printf("\n[ Tipo R ] \n");
        printf("opcode: %d\n", instrucao->opcode);
        printf("rs: %d\n", instrucao->rs);
        printf("rt: %d\n", instrucao->rt);
        printf("rd: %d\n", instrucao->rd);
        printf("funct: %d\n", instrucao->funct);
        break;

    case 2:
        instrucao->tipoInst = tipoJ;
        instrucao->addr = (instrucao->instrucao) &0xFF; // pega os 8 bits do adress
        printf("\n[ Tipo J ]\n");
        printf("opcode: %d\n", instrucao->opcode);
        printf("address: %d\n", instrucao->addr);
        break;

    default:
        instrucao->tipoInst = tipoI;
        instrucao->rs = (instrucao->instrucao >> 9) &0x7; // pega os 3 bits do rs
        instrucao->rt = (instrucao->instrucao >> 6) &0x7; // pega os 3 bits do rt
        instrucao->imm = (instrucao->instrucao) &0x3F; // pega os 6 bits do imediato (deve passar por um extensor antes da ULA)
        instrucao->imm = extensorBit(instrucao->imm);
        printf("\n[ Tipo I ] \n");
        printf("opcode: %d\n", instrucao->opcode);
        printf("rs: %d\n", instrucao->rs);
        printf("rt: %d\n", instrucao->rt);
        printf("imediato: %d\n", instrucao->imm);
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
    printf("________________________\n");
    printf(" Banco de Registradores \n");
    printf("________________________\n");
    printf(" Registrador |   Valor  \n");
    printf("________________________\n");

    for(int i=0;i<8;i++){
        printf("      %d      |     %d    \n",i, reg[i]);
        printf("_____________|__________\n");
    }
    printf("\n");
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
            printf("\nOperação da ULA inválida!\n");
    }
    //FECHA SE FOR OVERFLOW
    if(*overflow == 1){
    printf("\n OVERFLOW!\n");
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

void lerMemDados(char *arqMem, int **memDados) {
    int i=0;

    if (*memDados == NULL) {
        printf("\nErro ao alocar memória\n");
        return;
    }

    arquivoMemDados = fopen(arqMem, "r");
    if (arquivoMemDados == NULL) {
        printf("\nErro ao abrir o arquivo %s\n", arqMem);
        return;
    }

    for(i = 0; i < 256; i++) {
        fscanf(arquivoMemDados, "%d", &(*memDados)[i]);
    }

    printf("\nMemória carregada!\n");

    fclose(arquivoMemDados);
}

void escreveMemDados(int *memDados, int endereco, int8_t valor) {
    if (endereco >= 0 && endereco < 256) {
        memDados[endereco] = valor;
    } else {
        printf("\nErro ao escrever na memória.\n");
    }
}

int8_t retornaMemoria(int *memDados, uint8_t enderecoULA) {
    return memDados[enderecoULA];
}

void do_IF(IF_ID *out, instrucao *memoria, int *pc) {
    if(*pc >= 256 || memoria[*pc].instrucao == 0) {
        out->instrucao = 0;
        return;
    }
    out->pc = *pc;
    out->inst = memoria[*pc]; // <- guarda a instrução inteira
    (*pc)++;
}

void do_ID(ID_EX *out, IF_ID *in, int *bReg) {

    // decodifica normal
    instrucao inst = in->inst;
    decodificaInst(&inst);
    unidadeControle(&inst, &out->sinais);

    out->rs = inst.rs; out->rt = inst.rt; out->rd = inst.rd;
    out->imm = inst.imm; out->funct = inst.funct; out->opcode = inst.opcode;
    lerRegistradores(bReg, inst.rs, inst.rt, &out->A, &out->B);
}

void do_EX(ID_EX *in, EX_MEM *out) {

    int op2;
    int zero;
    int overflow;

    if(in->sinais.UlaFonte)
        op2 = in->imm;
    else
        op2 = in->B;

    out->ulaSaida =
        ULA(in->A,
            op2,
            in->sinais.ulaOp,
            &zero,
            &overflow);

    out->opcode = in->opcode;

    out->B = in->B;

    out->sinais = in->sinais;

    if(in->sinais.RegDst)
        out->rd = in->rd;
    else
        out->rd = in->rt;

}

void do_MEM(EX_MEM *in,
            MEM_WB *out,
            int *memDados)
{

    out->opcode = in->opcode;
    out->rd = in->rd;
    out->sinais = in->sinais;

    out->ulaSaida = in->ulaSaida;

    if(in->sinais.EscMem){
        escreveMemDados(
            memDados,
            in->ulaSaida,
            in->B
        );
    }

    if(in->sinais.MemParaReg == 0){
        out->mem =
            retornaMemoria(
                memDados,
                in->ulaSaida
            );
    }
}

void executaWB(MEM_WB *in, int *bReg, estatInstrucoes *estatInst) { // Mudar "in"
    
    printf("\n\n=============================================\n");
    printf("\n                    WB\n");
    printf("\n=============================================\n");
    
    if(in->sinais.EscReg == 1) {
        int8_t dadoFinal;
        
        if(in->sinais.MemParaReg == 1){
            dadoFinal = in->mem;
            printf("\n[ WB ] Dado %d preparado para escrita no banco de registradores. (Vindo da memória)\n", dadoFinal);
        }else{
            dadoFinal = in->ulaSaida;
            printf("\n[ WB ] Dado %d preparado para escrita no banco de registradores. (Vindo da ULA)\n", dadoFinal);
        }

        escreveRegistrador(bReg, in->rd, dadoFinal, in->sinais.EscReg);
        printf("\n[ WB ] %d escrito no registrador $%d.\n", dadoFinal, in->rd);
    }
    
    estatInst->total++;
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

void salvaASM(instrucao *memoria, int linhas) {
    int pc = 0;
    char nomeASM[50]={0}, nome[20], extensao[] = ".asm", resposta;

    printf("\nDigite o nome do arquivo que deseja salvar (.asm): ");
    fgets(nome, sizeof(nome),stdin);
    nome[strcspn(nome,"\n")]='\0';
    int indice=1;

    strcat(nomeASM,nome);
    strcat(nomeASM,extensao);

    // Verifica se o arquivo existe
    while (access(nomeASM, F_OK) != -1) {
        printf("\nJá existe um arquivo com o nome %s, deseja sobrescrever? (s/n): ", nomeASM);
        scanf(" %c", &resposta);

        if (resposta == 's' || resposta == 'S') {
            break;
        } else if (resposta == 'n' || resposta == 'N') {
            snprintf(nomeASM, sizeof(nomeASM), "%s_%d%s", nome, indice, extensao);
            indice++;
        } else {
            printf("\nOpção inválida. Tente novamente.\n");
        }
    }

    arquivo = fopen(nomeASM, "w");

    if (arquivo == NULL) {
        printf("\nErro ao criar arquivo\n");
        return;
    }

    while(pc < linhas){
        if((memoria)[pc].decodificado==0){
            decodificaInst(&memoria[pc]);
        }

        switch(memoria[pc].opcode){
            case 0: // opcode = 0000

                if(memoria[pc].funct==0){
                    fprintf(arquivo,"add $%d, $%d, $%d\n", (memoria)[pc].rd, (memoria)[pc].rs, (memoria)[pc].rt);
                }
                else if((memoria)[pc].funct==2){
                    fprintf(arquivo,"sub $%d, $%d, $%d\n", (memoria)[pc].rd, (memoria)[pc].rs, (memoria)[pc].rt);
                }
                else if((memoria)[pc].funct==4){
                    fprintf(arquivo,"and $%d, $%d, $%d\n", (memoria)[pc].rd, (memoria)[pc].rs, (memoria)[pc].rt);
                }
                else if((memoria)[pc].funct==5){
                    fprintf(arquivo,"or $%d, $%d, $%d\n", (memoria)[pc].rd, (memoria)[pc].rs, (memoria)[pc].rt);
                }
                break;

            case 2: // opcode = 0010 - J
                fprintf(arquivo,"j %d\n", (memoria)[pc].addr);

                break;

            case 4: // opcode = 0100 - Addi
                fprintf(arquivo,"addi $%d, $%d, %d\n", (memoria)[pc].rt, (memoria)[pc].rs, (memoria)[pc].imm);

                break;

            case 8: // opcode = 1000 - BEQ
                fprintf(arquivo,"beq $%d, $%d, %d\n", (memoria)[pc].rs, (memoria)[pc].rt, (memoria)[pc].imm);

                break;

            case 11: // opcode = 1011 - lw
                fprintf(arquivo,"lw $%d, %d($%d)\n", (memoria)[pc].rt, (memoria)[pc].imm, (memoria)[pc].rs);

                break;

            case 15: // opcode = 1111 - sw
                fprintf(arquivo,"sw $%d, %d($%d)\n", (memoria)[pc].rt, (memoria)[pc].imm, (memoria)[pc].rs);

                break;
        }

        pc++;
    }

    fclose(arquivo);

    printf("\nArquivo '%s' salvo!\n",nomeASM);
}

void salvaDAT(int *memDados){
    char nomeDAT[50]={0}, nome[20], extensao[] = ".dat", resposta;

    printf("\nDigite o nome do arquivo que deseja salvar (.dat): ");
    fgets(nome, sizeof(nome),stdin);
    nome[strcspn(nome,"\n")]='\0';
    int indice=1;

    strcat(nomeDAT,nome);
    strcat(nomeDAT,extensao);

    // Verifica se o arquivo existe
    while (access(nomeDAT, F_OK) != -1) {
        printf("\nJá existe um arquivo com o nome %s, deseja sobrescrever? (s/n): ", nomeDAT);
        scanf(" %c", &resposta);

        if (resposta == 's' || resposta == 'S') {
            break;
        } else if (resposta == 'n' || resposta == 'N') {
            snprintf(nomeDAT, sizeof(nomeDAT), "%s_%d%s", nome, indice, extensao);
            indice++;
        } else {
            printf("\nOpção inválida. Tente novamente.\n");
        }
    }

    arquivo = fopen(nomeDAT,"w");

    if (arquivo == NULL) {
        printf("\nErro ao criar arquivo\n");
        return;
    }

    for(int i=0;i<256;i++){
        fprintf(arquivo,"%d\n",memDados[i]);
    }

    fclose(arquivo);

    printf("\nArquivo '%s' salvo!\n",nomeDAT);
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


void imprimeEstatistica(estatInstrucoes estatInst){
    estatInst.CPI = 0;
    if(estatInst.total > 0){
        estatInst.CPI = (float)estatInst.ciclos / (float)estatInst.total;
    }

    printf("\n========================================\n");
    printf("      Estatísticas do Simulador:\n");
    printf("========================================\n");

    printf("\nTotal executadas: %d\n", estatInst.total);
    printf("Ciclos: %d\n", estatInst.ciclos);

    printf("CPI (Ciclos por Instrução): %.2f\n", estatInst.CPI);
    printf("Stalls: %d\n", estatInst.stalls);

    printf("\nEstatísticas de instruções\n");
    printf("\nPor tipo:\n");
    printf("Tipo R: %d\n", estatInst.tipoR);
    printf("Tipo I: %d\n", estatInst.tipoI);
    printf("Tipo J: %d\n", estatInst.tipoJ);

    printf("\nDetalhamento por instrução:\n");
    printf("R -> add: %d | sub: %d | and: %d | or: %d\n",
           estatInst.add, estatInst.sub, estatInst.and, estatInst.or);
    printf("I -> addi: %d | beq: %d | lw: %d | sw: %d\n",
           estatInst.addi, estatInst.beq, estatInst.lw, estatInst.sw);
    printf("J -> j: %d\n", estatInst.j);


    printf("========================================\n\n");

    /*  NCurse
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
    
    getch();*/


}

void salvaEstado(historico *hist, int pc, int *memDados, int *bReg, estatInstrucoes *estatInst){
    if(hist->topo >= MAX_HIST) return;

    estado *e = &hist->estados[hist->topo];

    e->pc = pc;

    for(int i=0;i<256;i++)
        e->memDados[i] = memDados[i];

    for(int i=0;i<8;i++)
        e->bReg[i] = bReg[i];

    e->estat = *estatInst; // <-- SALVA ESTATÍSTICAS

    hist->topo++;
}

void voltaInstrucao(historico *hist, int *pc, int *memDados, int *bReg, estatInstrucoes *estatInst){
    if(hist->topo <= 0){
        printf("\nSem histórico!\n");
        return;
    }

    hist->topo--;

    estado *e = &hist->estados[hist->topo];

    *pc = e->pc;

    for(int i=0;i<256;i++)
        memDados[i] = e->memDados[i];

    for(int i=0;i<8;i++)
        bReg[i] = e->bReg[i];

    *estatInst = e->estat; // <-- RESTAURA ESTATÍSTICAS

    printf("\nVoltou uma instrução!\n");
    printf("PC atual: %d.\n", *pc);
}

void imprimeInstrucao(instrucao *memoria, int pc) {
    if((memoria)[pc].decodificado==0){
        decodifica(&memoria[pc]);
    }

    switch(memoria[pc].opcode){
        case 0: // opcode = 0000

            if(memoria[pc].funct==0){
                printf("add $%d, $%d, $%d", (memoria)[pc].rd, (memoria)[pc].rs, (memoria)[pc].rt);
            }
            else if((memoria)[pc].funct==2){
                printf("sub $%d, $%d, $%d", (memoria)[pc].rd, (memoria)[pc].rs, (memoria)[pc].rt);
            }
            else if((memoria)[pc].funct==4){
                printf("and $%d, $%d, $%d", (memoria)[pc].rd, (memoria)[pc].rs, (memoria)[pc].rt);
            }
            else if((memoria)[pc].funct==5){
                printf("or $%d, $%d, $%d", (memoria)[pc].rd, (memoria)[pc].rs, (memoria)[pc].rt);
            }
            break;

        case 2: // opcode = 0010 - J
            int x=12;
            printf("j %d%*s", (memoria)[pc].addr, x, "");

            break;

        case 4: // opcode = 0100 - Addi
            printf("addi $%d, $%d, %d", (memoria)[pc].rt, (memoria)[pc].rs, (memoria)[pc].imm);

            break;

        case 8: // opcode = 1000 - BEQ
            printf("beq $%d, $%d, %d", (memoria)[pc].rs, (memoria)[pc].rt, (memoria)[pc].imm);

            break;

        case 11: // opcode = 1011 - lw
            printf("lw $%d, %d($%d)", (memoria)[pc].rt, (memoria)[pc].imm, (memoria)[pc].rs);

            break;

        case 15: // opcode = 1111 - sw
            printf("sw $%d, %d($%d)", (memoria)[pc].rt, (memoria)[pc].imm, (memoria)[pc].rs);

            break;
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

int eh_bolha(sinaisUC sinais) {
    if (sinais.EscReg || sinais.EscMem || sinais.branch || sinais.jump) {
        return 0; // Tem algum sinal ativo, então não é bolha
    }
    return 1; // Tudo zero, É uma bolha
}
// TODO: Ajustar essas impressões - ncurses
void print_pipeline_state(registradoresPipeline *pipe, int ciclo) {
    printf("\n========== Ciclo %d ==========\n", ciclo);

    int ocupados = 0;

    // IF
    if(pipe->regIF_ID_atual.inst.instrucao!=0) {
        printf("IF : PC=%d opcode=%d\n", pipe->regIF_ID_atual.pc, pipe->regIF_ID_atual.inst.opcode);
        ocupados++;
    }
    else{
        printf("\nIF: Stall\n");
    }

    // ID
    if(!eh_bolha(pipe->regID_EX_atual.sinais)) {
        printf("ID : opcode=%d rs=%d rt=%d rd=%d A=%d B=%d\n",
            pipe->regID_EX_atual.opcode, pipe->regID_EX_atual.rs, pipe->regID_EX_atual.rt,
            pipe->regID_EX_atual.rd, pipe->regID_EX_atual.A, pipe->regID_EX_atual.B);
        ocupados++;
    }
    else{
        printf("\nID: Stall\n");
    }

    // EX
    if(!eh_bolha(pipe->regEX_MEM_atual.sinais)) {
        printf("EX : opcode=%d ulaSaida=%d rd=%d\n",
            pipe->regEX_MEM_atual.opcode, pipe->regEX_MEM_atual.ulaSaida, pipe->regEX_MEM_atual.rd);
        ocupados++;
    }
    else{
        printf("\nEX: Stall\n");
    }

    // MEM/WB - junta os dois pq WB só escreve o que veio de MEM
    if(!eh_bolha(pipe->regMEM_WB_atual.sinais)) {
        printf("MEM: opcode=%d ulaSaida=%d mem=%d rd=%d\n",
            pipe->regMEM_WB_atual.opcode, pipe->regMEM_WB_atual.ulaSaida,
            pipe->regMEM_WB_atual.mem, pipe->regMEM_WB_atual.rd);
        printf("WB : opcode=%d vai escrever rd=%d\n",
            pipe->regMEM_WB_atual.opcode, pipe->regMEM_WB_atual.rd);
        ocupados++;
    }
    else{
        printf("\nMEM: Stall\n");
        printf("\nWB: Stall\n");
    }

    printf("Instruções no pipeline: %d/4\n", ocupados); // 4 instruções?
    printf("============================\n\n");
}


/*
void step(instrucao *memoria, int *bReg, sinaisUC *sinais, int *pc, int *memDados, estatInstrucoes *estatInst){

    if(*pc>=256 || memoria[*pc].instrucao==0){
        printf("\nFim das instruções!\n");
        if(memoria[*pc].instrucao==0)
            printf("\nMotivo: NOP (instrução 0000000000000000)\n");
        else
            printf("\nMotivo: Uso total da memória.\n");
        return;
    }

    printf("\nPC = %d | Memória = %s\n", *pc, memoria[*pc].mem);

    // Decodifica
    decodificaInst(&memoria[*pc]);

    memoria[*pc].decodificado = 1;
    // Contabiliza estatística
    contabilizaEstat(memoria, estatInst, *pc);

    // Controle
    unidadeControle(&memoria[*pc], sinais);

    // Executa
    int zero = executaInstrucao(&memoria[*pc], sinais, bReg, memDados);

    //PC
    programCounter(pc, sinais, &memoria[*pc], zero);

    (*estatInst).total++;
}

int executaInstrucao(instrucao* instrucao, sinaisUC *sinais, int *bReg, int *memDados){
    int8_t  operador1, operador2, UlaResultado=0, regDst, dadoFinal=0, valorSW;
    int zero=0, overflow = 0;
    lerRegistradores(bReg, (*instrucao).rs, (*instrucao).rt, &operador1, &operador2);

    valorSW = operador2;

    if((*sinais).UlaFonte==1){
        operador2=(*instrucao).imm;
    }

    UlaResultado = ULA(operador1, operador2, (*sinais).ulaOp, &zero, &overflow);

    if((*sinais).EscMem==1){
        escreveMemDados(memDados, (int)UlaResultado, valorSW);
        printf("\nSW: Valor %d guardado no endereço %d\n", valorSW, UlaResultado);
    }

    if((*sinais).MemParaReg==1){
        dadoFinal = UlaResultado;
    }else if((*sinais).EscReg==1){
        dadoFinal = retornaMemoria(memDados, (uint8_t)UlaResultado);
        printf("\nLW: Valor %d lido do endereço %d\n", dadoFinal, UlaResultado);
    }

    if((*sinais).RegDst==1){
        regDst = (*instrucao).rd;
    }else{
        regDst = (*instrucao).rt;
    }

    if((*sinais).EscReg==1){
        escreveRegistrador(bReg, regDst, dadoFinal, (*sinais).EscReg);
        printf("\nRegistrador a ser escrito: $%d com o valor %d\n", regDst, dadoFinal);
    }

    if((*sinais).branch==1 && zero == 1){
        printf("\nPulo condicional detectado\n");
    }
    return zero;
}

void run(instrucao *memoria, int *bReg, sinaisUC *sinais, int *pc, int *memDados, estatInstrucoes *estatInst){

    while(*pc < 256 && memoria[*pc].instrucao!=0){

        printf("\nPC = %d | Memória = %s\n", *pc, memoria[*pc].mem);

        // Decodifica
        decodificaInst(&memoria[*pc]);

        memoria[*pc].decodificado = 1;
        // Contabiliza estatística
        contabilizaEstat(memoria, estatInst, *pc);

        // Controle
        unidadeControle(&memoria[*pc], sinais);

        // Executa
        int zero = executaInstrucao(&memoria[*pc], sinais, bReg, memDados);

        programCounter(pc, sinais, &memoria[*pc], zero);

        (*estatInst).total++;
    }

    if(*pc>=256 || memoria[*pc].instrucao==0){
        printf("\nFim das instruções!\n");
        if(memoria[*pc].instrucao==0)
            printf("\nMotivo: HALT (instrução 0000000000000000)\n");
        else
            printf("\nMotivo: Uso total da memória.\n");
        return;
    }
}

void carregaID_EX(instrucao *inst, int *bReg, registradoresPipeline *pipe){

    sinaisUC sinais;

    decodificaInst(inst); //talvez tirar esse decodifica daqui para melhor representação do simulador

    unidadeControle(inst, &sinais); //essa chamada da UC aqui também

    pipe->regID_EX.sinais = sinais;

    pipe->regID_EX.opcode = inst->opcode;
    pipe->regID_EX.rs = inst->rs;
    pipe->regID_EX.rt = inst->rt;
    pipe->regID_EX.rd = inst->rd;
    pipe->regID_EX.funct = inst->funct;
    pipe->regID_EX.imm = inst->imm;

    pipe->regID_EX.A = bReg[inst->rs];
    pipe->regID_EX.B = bReg[inst->rt];
}

void carregaEX_MEM(registradoresPipeline *pipe, int8_t resultadoULA){

    pipe->regEX_MEM.sinais = pipe->regID_EX.sinais;

    pipe->regEX_MEM.opcode = pipe->regID_EX.opcode;

    pipe->regEX_MEM.ulaSaida = resultadoULA;

    pipe->regEX_MEM.B = pipe->regID_EX.B;

    if(pipe->regID_EX.sinais.RegDst){
        pipe->regEX_MEM.rd = pipe->regID_EX.rd;
    }else{
        pipe->regEX_MEM.rd = pipe->regID_EX.rt;
    }
}

void carregaMEM_WB(registradoresPipeline *pipe, int8_t dadoMemoria){

    pipe->regMEM_WB.sinais = pipe->regEX_MEM.sinais;

    pipe->regMEM_WB.opcode = pipe->regEX_MEM.opcode;

    pipe->regMEM_WB.rd = pipe->regEX_MEM.rd;

    pipe->regMEM_WB.ulaSaida = pipe->regEX_MEM.ulaSaida;

    pipe->regMEM_WB.mem = dadoMemoria;

}

*/
