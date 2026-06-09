#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define MAX_HIST 1000

 extern FILE *arquivo, *arquivoMemDados;

// struct das estatísticas
typedef struct{
    // estatísticas das instruções
    int tipoI;
    int tipoJ;
    int tipoR;
    int total;
    int add, sub, and, or, addi, beq, lw, sw, j;

    // estatísticas de ciclos, CPI e stalls até o momento
    int ciclos, stalls/*, hazards*/;
    float CPI;
} estatInstrucoes;

// struct de sinais
typedef struct{
    uint8_t branch;
    uint8_t jump;
    uint8_t IncPC;
    uint8_t RegDst;
    uint8_t UlaFonte;
    uint8_t MemParaReg;
    uint8_t EscReg;
    uint8_t EscMem;
    uint8_t ulaOp;
}sinaisUC;

// struct das instruções
enum inst{
    tipoI, tipoJ, tipoR
};

typedef struct {
    char mem[17];
    enum inst tipoInst;
    uint16_t instrucao;
    uint8_t opcode;
    uint8_t rs;
    uint8_t rt;
    uint8_t rd;
    uint8_t funct;
    int8_t imm;
    uint8_t addr;
    int decodificado;
} instrucao;

typedef struct{
    int pc;
    uint16_t instrucao;
    int valido;
    instrucao inst;
}IF_ID;

typedef struct{
    sinaisUC sinais;
    uint8_t opcode;
    int8_t A; // RS
    int8_t B; // RT
    uint8_t rs;
    uint8_t rt; // Segue pois pode ser utilizado como destino no WB
    uint8_t rd;
    uint8_t funct;
    int8_t imm;
    int valido;
}ID_EX;

typedef struct{
    sinaisUC sinais; // RegDst, UlaFont e ControleUla ficam
    uint8_t opcode;
    int8_t ulaSaida;
    int8_t B; // Valor escrito na memória em um store (RT)
    uint8_t rd;
    int valido;
}EX_MEM;

typedef struct{
    sinaisUC sinais; // Branch, Jump e EscMem ficam
    uint8_t opcode;
    int8_t mem;
    int8_t ulaSaida;
    uint8_t rd;
    int valido;
}MEM_WB;

typedef struct{
    IF_ID regIF_ID_atual, regIF_ID_novo;
    ID_EX regID_EX_atual, regID_EX_novo;
    EX_MEM regEX_MEM_atual, regEX_MEM_novo;
    MEM_WB regMEM_WB_atual, regMEM_WB_novo;
}registradoresPipeline;

typedef struct {
    int pc;
    int memDados[256];
    int bReg[8];
    estatInstrucoes estat;
} estado;

typedef struct {
    estado estados[MAX_HIST];
    int topo;
} historico;

/*Instruções:
Tipo R:
opcode: 4 bits (0000);
rs: 3 bits;
rt: 3 bits;
rd: 3 bits;
funct: 3 bits

Tipo I:
opcode: 4 bits;
rs (base/registrador fonte): 3 bits;
rt (destino/registrador fonte): 3 bits;
imediato: 6 bits (estendido);

Tipo J:
opcode: 4 bits;
End: 8 bits;
*/

// ------------------------------ PROTÓTIPOS -------------------------------

// MENU / CONTROLE DO SISTEMA
void contabilizaEstat(instrucao *memoria, estatInstrucoes *estat, int pc);
void imprimeEstatistica(estatInstrucoes estatInst);
void salvaASM(instrucao *memoria, int linhas);
void salvaDAT(int *memDados);
void run_pipeline(instrucao *memoria, int *bReg, int *pc, int *memDados, registradoresPipeline *pipe, estatInstrucoes *estatInst);
void step_pipeline(instrucao *memoria, int *bReg, int *pc, int *memDados, registradoresPipeline *pipe, estatInstrucoes *estatInst);
/*
void carregaID_EX(instrucao *inst, int *bReg, registradoresPipeline *pipe);
void carregaEX_MEM(registradoresPipeline *pipe, int8_t resultadoULA);
void carregaMEM_WB(registradoresPipeline *pipe, int8_t dadoMemoria);
*/

// MEMÓRIA
int contaLinhas(char *arq);
void lerMem(char *arq, instrucao **memoria, int linhas);
void imprimeMemorias(instrucao *memoria, int *memDados);
void imprimeInstrucao(instrucao *memoria, int pc);
void decodifica(instrucao *instrucao);

// PROGRAM COUNTER (PC) / BUSCA
void programCounter(int *pc, sinaisUC *sinais, instrucao *instrucao, int zero);


// DECODIFICAÇÃO
void decodificaInst(instrucao *instrucao);


// UNIDADE DE CONTROLE (UC)
void unidadeControle(instrucao *instrucao, sinaisUC *sinais);


// BANCO DE REGISTRADORES (BREG)
int *inicializaBReg();
void lerRegistradores(int *reg, int8_t rs, int8_t rt, int8_t *valRs, int8_t *valRt);
void escreveRegistrador(int *reg, int8_t rd, int8_t valor, int EscReg);
void imprimeBancoRegistradores(int *reg);


// EXECUÇÃO
int executaInstrucao(instrucao *instrucao, sinaisUC *sinais, int *bReg, int *memDados);
int8_t extensorBit(int8_t imm);
void atualiza_regs_pipeline(registradoresPipeline *pipe);
void print_pipeline_state(registradoresPipeline *pipe, int ciclo);

// ULA (UNIDADE LÓGICA E ARITMÉTICA)
int8_t ULA(int op1, int op2, int ulaOp, int *zero, int *overflow);

// MEMÓRIA DE DADOS
int *inicializaMemDados();
void lerMemDados(char *arqMem, int **memDados);
void escreveMemDados(int *memDados, int endereco, int8_t valor);
int8_t retornaMemoria(int *memDados, uint8_t enderecoULA);

// HISTÓRICO
void salvaEstado(historico *hist, int pc, int *memDados, int *bReg, estatInstrucoes *estatInst);
void voltaInstrucao(historico *hist, int *pc, int *memDados, int *bReg, estatInstrucoes *estatInst);

// ESTÁGIOS
void do_IF(IF_ID *out, instrucao *memoria, int *pc);
void do_ID(ID_EX *out, IF_ID *in, int *bReg);
void do_EX(ID_EX *in, EX_MEM *out);
void do_MEM(EX_MEM *in, MEM_WB *out, int *memDados);
void do_WB(MEM_WB *in, int *bReg, estatInstrucoes *estatInst);
void atualiza_regs_pipeline(registradoresPipeline *pipe);
void inicializa_pipeline(registradoresPipeline *pipe);
