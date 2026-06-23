#ifndef MINIMIPS_H
#define MINIMIPS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <ncurses.h>

extern FILE *arquivo, *arquivoMemDados;

// struct das estatísticas
typedef struct {
    int tipoI;
    int tipoJ;
    int tipoR;
    int total;
    int add, sub, and, or, addi, beq, lw, sw, j;
    int ciclos, stalls;
    float CPI;
} estatInstrucoes;

// struct de sinais
typedef struct {
    uint8_t branch;
    uint8_t jump;
    uint8_t IncPC;
    uint8_t RegDst;
    uint8_t UlaFonte;
    uint8_t MemParaReg;
    uint8_t EscReg;
    uint8_t EscMem;
    uint8_t ulaOp;
} sinaisUC;

enum inst { tipoI, tipoJ, tipoR };

// struct das instruções
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

typedef struct IF_ID {
    int pc;
    uint16_t instrucao;
    instrucao inst;
} IF_ID;

typedef struct ID_EX {
    sinaisUC sinais;
    uint8_t opcode;
    int8_t A;
    int8_t B;
    uint8_t rs;
    uint8_t rt;
    uint8_t rd;
    uint8_t funct;
    int8_t imm;
} ID_EX;

typedef struct EX_MEM {
    sinaisUC sinais;
    uint8_t opcode;
    int8_t ulaSaida;
    int8_t B;
    uint8_t rd;
    int8_t zero;
} EX_MEM;

typedef struct MEM_WB {
    sinaisUC sinais;
    uint8_t opcode;
    int8_t mem;
    int8_t ulaSaida;
    uint8_t rd;
} MEM_WB;

typedef struct {
    int flushIF;
    int flushID;
    int stallIF;
    int stallID;
} controlePipeline;

typedef struct {
    controlePipeline ctrl;
    IF_ID regIF_ID_atual, regIF_ID_novo;
    ID_EX regID_EX_atual, regID_EX_novo;
    EX_MEM regEX_MEM_atual, regEX_MEM_novo;
    MEM_WB regMEM_WB_atual, regMEM_WB_novo;
} registradoresPipeline;

// Estrutura completa do Estado do Simulador
typedef struct {
    int pc;
    int memDados[256];
    int bReg[8];
    estatInstrucoes estat;
    registradoresPipeline pipe;
} estado;

// Pilha Dinâmica para o Step-Back
typedef struct Node {
    estado st;
    struct Node *next;
} Node;

typedef struct {
    Node *topo;
} historico;

// Protótipos das Novas Funções e Modificadas
void salvaEstado(historico *hist, int pc, int *memDados, int *bReg, estatInstrucoes *estatInst, registradoresPipeline *pipe);
void voltaInstrucao(historico *hist, int *pc, int *memDados, int *bReg, estatInstrucoes *estatInst, registradoresPipeline *pipe);
void liberaHistorico(historico *hist);

void imprimeBancoRegistradores(int *reg);
char *imprimeInstrucao(instrucao *memoria, int pc);

int contaLinhas(char *arq);
int lerMem(int colunaspainel, int linhaspainel, char *arq, instrucao **memoria, int linhas);
int lerMemDados(int linhaspainel, int colunaspainel, char *arqMem, int **memDados);
void imprimeMemorias(int colunaspainel, int linhaspainel, instrucao *memoria, int *memDados);
int *inicializaBReg();
int *inicializaMemDados();

void step_pipeline(historico *hist, instrucao *memoria, int *bReg, int *pc, int *memDados, registradoresPipeline *pipe, estatInstrucoes *estatInst);
void run_pipeline(historico *hist, instrucao *memoria, int *bReg, int *pc, int *memDados, registradoresPipeline *pipe, estatInstrucoes *estatInst);
void atualiza_regs_pipeline(registradoresPipeline *pipe);

void Executa_IF(IF_ID *IF_ID, instrucao *memoria, int *pc);
void Executa_ID(ID_EX *ID_EX, IF_ID *IF_ID, int *bReg);
void Executa_EX(ID_EX *ID_EX, EX_MEM *EX_MEM, MEM_WB *MEM_WB_atual);
void Executa_MEM(EX_MEM *EX_MEM, MEM_WB *MEM_WB, int *memDados);
void Executa_WB(MEM_WB *MEM_WB, int *bReg);

void insereStall(registradoresPipeline *pipe, estatInstrucoes *estat);
void insereFlush(registradoresPipeline *pipe);
void decodificaInst(instrucao *instrucao);
void unidadeControle(instrucao *instrucao, sinaisUC *sinais);
int8_t ULA(int op1, int op2, int ulaOp, int *zero, int *overflow);
int8_t extensorBit(int8_t imm);
int Verifica_Bolha(sinaisUC sinais);
void ajustarPC(int *pc, int novoPC);
void forwardingUnit(ID_EX *ID_EX, EX_MEM *EX_MEM, MEM_WB *MEM_WB, uint8_t *forwardA, uint8_t *forwardB);

void printBorda(int linhas, int colunas);
void salvaASM(int colunaspainel, int linhaspainel, instrucao *memoria, int linhas);
void salvaDAT(int *memDados);

//IMPRIMIR TODO SIMULADOR
void imprimeTodoSimulador(int colunaspainel, int linhaspainel, registradoresPipeline *pipe, estatInstrucoes *estatInst, int *bReg, int pc, instrucao *memoria, int *memDados, historico *hist);
char *nomeInstrucao(uint8_t opcode, uint8_t funct);
char *nomeULA(uint8_t ulaOp);



#include "hazards.h"

#endif