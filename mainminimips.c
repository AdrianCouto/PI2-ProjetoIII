#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include "minimips.h"

static void desenhaInterfacePrincipal(int colunaspainel, int linhaspainel, registradoresPipeline *pipe, estatInstrucoes *estatInst, int *bReg, int pc, instrucao *memoria, historico *hist, int *memDados, int selecionado, char *opcoesExec[], char *opcoesMenu[], int ultimoHazard);  

static void iniciaNcurses(void){
    initscr();
    start_color();
    use_default_colors();
    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, COLOR_YELLOW, -1);
    init_pair(3, COLOR_GREEN, -1);
    init_pair(4, COLOR_RED, -1);
    init_pair(5, COLOR_BLUE, -1);
    init_pair(6, COLOR_MAGENTA, -1);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
}

int main(){
    setlocale(LC_ALL, "");
    iniciaNcurses();
    mousemask(0, NULL);

    int pc = 0, opcao = -1, linhas = 0;
    estatInstrucoes estatInst = {0};
    instrucao *memoria = NULL;
    historico hist;
    hist.topo = 0;

    int *bReg = inicializaBReg();
    int *memDados = inicializaMemDados();
    registradoresPipeline pipe = {0};

    int selecionado = 0;
    int tecla;
    
    int ultimoHazard = 0;

    char *opcoesExec[] = { 
        "Run Pipeline", 
        "Step Pipeline", 
        "Step Back" 
    };

    char *opcoesMenu[] = {
        "Carregar Instruções (.mem)",
        "Carregar Dados (.dat)",
        "Visualizar Memórias",
        "Salvar .asm",
        "Salvar .dat",
        "Sair do Programa"
    };

    int linhaspainel, colunaspainel;

    while (1) {
        getmaxyx(stdscr, linhaspainel, colunaspainel);

        desenhaInterfacePrincipal(colunaspainel, linhaspainel, &pipe, &estatInst, bReg, pc, memoria, &hist, memDados, selecionado, opcoesExec, opcoesMenu, ultimoHazard);

        tecla = getch();
        opcao = -1;

        switch(tecla) {
            case KEY_LEFT:
                if(selecionado > 0 && selecionado <= 2) selecionado--;
                else if(selecionado >= 3) selecionado = 2;
                continue;

            case KEY_RIGHT:
                if(selecionado >= 0 && selecionado < 2) selecionado++;
                else if(selecionado == 2) selecionado = 3; 
                continue;

            case KEY_UP:
                if(selecionado > 3) selecionado--;
                else if(selecionado == 3) selecionado = 8;
                else if(selecionado <= 2) selecionado = 3;
                continue;

            case KEY_DOWN:
                if(selecionado >= 3 && selecionado < 8) selecionado++;
                else if(selecionado == 8) selecionado = 3;
                else if(selecionado <= 2) selecionado = 3;
                continue;

            case 10: // Tecla ENTER
            case KEY_ENTER:
                opcao = selecionado;
                break;
        }

        if (opcao == -1) continue;

        switch(opcao) {
            case 0:
                while(1){
                    ultimoHazard = step_pipeline(&hist, memoria, bReg, &pc, memDados, &pipe, &estatInst);
                    
                    desenhaInterfacePrincipal(colunaspainel, linhaspainel, &pipe, &estatInst, bReg, pc, memoria, &hist, memDados, selecionado, opcoesExec, opcoesMenu, ultimoHazard);
                
                    if ((pc >= 256 || (memoria != NULL && memoria[pc].instrucao == 0)) && 
                        pipe.regIF_ID_atual.inst.instrucao == 0 && 
                        pipe.regID_EX_atual.opcode == 0 && 
                        pipe.regEX_MEM_atual.opcode == 0 && 
                        pipe.regMEM_WB_atual.opcode == 0) {
                        break;
                    }
                    usleep(1000000);
                }
                ultimoHazard = 0;
                break;

            case 1: // STEP 
                ultimoHazard = step_pipeline(&hist, memoria, bReg, &pc, memDados, &pipe, &estatInst); 
                break;

            case 2: // STEP BACK
                voltaInstrucao(&hist, &pc, memDados, bReg, &estatInst, &pipe); 
                ultimoHazard = 0;
                break;

            case 3: { // Carregar .mem
                char arq[20];
                clear();
                printBorda(linhaspainel, colunaspainel);
                attron(COLOR_PAIR(3) | A_BOLD);
                mvprintw((linhaspainel/2), (colunaspainel/2) - 20, "Digite o nome do arquivo .mem: ");
                attroff(COLOR_PAIR(3) | A_BOLD);

                echo(); curs_set(1);
                mvgetnstr((linhaspainel/2), (colunaspainel/2) + 11, arq, sizeof(arq)-1);
                noecho(); curs_set(0);

                linhas = contaLinhas(arq);
                if (lerMem(colunaspainel, linhaspainel, arq, &memoria, linhas)) {
                    clear(); printBorda(linhaspainel, colunaspainel);
                    mvprintw((linhaspainel/2), (colunaspainel/2) - 10, "Arquivo carregado!");
                } else {
                    clear(); printBorda(linhaspainel, colunaspainel);
                    mvprintw((linhaspainel/2), (colunaspainel/2) - 12, "Falha ao carregar arquivo!");
                }
                refresh(); getch();
                break;
            }

            case 4: { // Carregar .dat
                char arqMem[20];
                clear();
                printBorda(linhaspainel, colunaspainel);
                attron(COLOR_PAIR(3) | A_BOLD);
                mvprintw((linhaspainel/2), (colunaspainel/2) - 20, "Digite o nome do arquivo .dat: ");
                attroff(COLOR_PAIR(3) | A_BOLD);

                echo(); curs_set(1);
                mvgetnstr((linhaspainel/2), (colunaspainel/2) + 11, arqMem, sizeof(arqMem)-1);
                noecho(); curs_set(0);

                if(lerMemDados(linhaspainel, colunaspainel, arqMem, &memDados)){
                    clear(); printBorda(linhaspainel, colunaspainel);
                    mvprintw((linhaspainel/2), (colunaspainel/2) - 10, "Arquivo carregado!");
                } else {
                    clear(); printBorda(linhaspainel, colunaspainel);
                    mvprintw((linhaspainel/2), (colunaspainel/2) - 12, "Falha ao carregar arquivo!");
                }
                refresh(); getch();
                break;
            }

            case 5: // Visualizar Memorias
                endwin();
                imprimeMemorias(colunaspainel, linhaspainel, memoria, memDados);
                iniciaNcurses();
                break;

            case 6: // Salvar .asm
                salvaASM(colunaspainel, linhaspainel, memoria, linhas);
                iniciaNcurses();
                break;

            case 7: // Salvar .dat
                endwin();
                salvaDAT(memDados);
                iniciaNcurses();
                break;

            case 8: // Sair
                endwin();
                liberaHistorico(&hist);
                free(bReg); bReg = NULL;
                free(memDados); memDados = NULL;
                if(memoria != NULL) { free(memoria); memoria = NULL; }
                printf("\nSaindo do simulador...\n");
                return 0;
        }
    }
    return 0;
}

static void desenhaInterfacePrincipal(int colunaspainel, int linhaspainel, registradoresPipeline *pipe, estatInstrucoes *estatInst, int *bReg, int pc, instrucao *memoria, historico *hist, int *memDados, int selecionado, char *opcoesExec[], char *opcoesMenu[], int ultimoHazard) {    
    clear();
    printBorda(linhaspainel, colunaspainel);

    attron(A_BOLD | COLOR_PAIR(1));
    mvprintw(0, colunaspainel/2 - 11, " SIMULADOR DE PIPELINE ");
    attroff(A_BOLD | COLOR_PAIR(1));

    if (ultimoHazard > 0) {
        attron(A_BOLD | A_BLINK | COLOR_PAIR(4));
        if (ultimoHazard == 1) {
            mvprintw(0, 4, " [ HAZARD DE DADOS ] ");
        } else if (ultimoHazard == 2) {
            mvprintw(0, 4, " [ HAZARD DE CONTROLE ] ");
        }
        attroff(A_BOLD | A_BLINK | COLOR_PAIR(4));
    }

    attron(A_BOLD | COLOR_PAIR(1));

    mvhline(1, 2, ACS_HLINE, colunaspainel - 4);// primeira linha
    
    mvhline(linhaspainel / 3, 2, ACS_HLINE, colunaspainel - 4);// segunda linha
    mvhline(linhaspainel / 3 + 1, 2, ACS_HLINE, colunaspainel - 4);
    mvhline(linhaspainel / 3 - 2, 2, ACS_HLINE, colunaspainel - 4);

    mvhline((2 * linhaspainel)/3 + 1, 2, ACS_HLINE, colunaspainel - 4);//terceira linha
    mvhline((linhaspainel/3) * 2, 2, ACS_HLINE, colunaspainel - 4);
    
    mvhline(linhaspainel - 4, 2, ACS_HLINE, (colunaspainel - (colunaspainel/5)) - 4);// quarta linha
    mvhline(linhaspainel - 3, 2, ACS_HLINE, (colunaspainel - (colunaspainel/5)) - 4);
    
    mvvline(1, 2, ACS_VLINE, linhaspainel - 2);//pimeira coluna
    mvvline(1, colunaspainel / 5 - 1, ACS_VLINE, linhaspainel/3 - 1);
    
    mvvline(1, colunaspainel / 5, ACS_VLINE, linhaspainel/3);//segunda coluna
    mvvline(1, (colunaspainel / 5) * 2, ACS_VLINE, linhaspainel/3);
    mvvline(1, (colunaspainel / 5) * 2 - 1, ACS_VLINE, linhaspainel/3);
    
    mvvline(1, (colunaspainel / 5) * 3, ACS_VLINE, linhaspainel/3);//terceira coluna
    mvvline(1, (colunaspainel / 5) * 3 - 1, ACS_VLINE, linhaspainel/3);

    mvvline(1, (colunaspainel / 5) * 4, ACS_VLINE, linhaspainel/3);//quarta coluna 
    mvvline(1, (colunaspainel / 5) * 4 - 1, ACS_VLINE, linhaspainel/3);

    mvvline((linhaspainel/3) * 2 + 1, (colunaspainel/5) * 4 + 1, ACS_VLINE, linhaspainel - 41);

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
    mvaddch((linhaspainel/3) * 2 + 1, (colunaspainel/5) * 4, ACS_URCORNER);

    //cantos menu inferior
    mvaddch((linhaspainel / 3) * 2 + 1, 2, ACS_ULCORNER);
    mvaddch(linhaspainel - 3, (colunaspainel/5) * 4 + 1, ACS_LRCORNER);
    mvaddch((linhaspainel / 3) * 2 + 1, colunaspainel - 3, ACS_URCORNER);
    mvaddch((linhaspainel/3) * 2 + 1, (colunaspainel / 5) * 4 + 1, ACS_ULCORNER);
    mvaddch(linhaspainel - 4, 2, ACS_LLCORNER);
    mvaddch(linhaspainel - 3, 2, ACS_ULCORNER);

    attroff(A_BOLD | COLOR_PAIR(1));

    attron(A_BOLD | COLOR_PAIR(2)); // IF
    mvprintw(1, (colunaspainel/5) - colunaspainel/10 - 2, "  IF  ");
    mvprintw(3, (colunaspainel/5)/2 - 5,"PC : %d",pipe->regIF_ID_atual.pc);
    mvprintw(4, ((colunaspainel/5)/2 - 5),"HEX: %04X",pipe->regIF_ID_atual.inst.instrucao);
    if(memoria != NULL) mvprintw(6, ((colunaspainel/5)/2 - 5),"BIN: %s",pipe->regIF_ID_atual.inst.mem);
    attroff(A_BOLD | COLOR_PAIR(2));
    
    attron(A_BOLD | COLOR_PAIR(3)); // ID
    mvprintw(1, (colunaspainel/5 * 2) - colunaspainel/10 - 3, "  ID  ");
    mvprintw(3, (colunaspainel/5 * 2) - 30,"Instrução : %s", nomeInstrucao(pipe->regID_EX_atual.opcode,pipe->regID_EX_atual.funct));
    mvprintw(4, (colunaspainel/5 * 2) - 30,"rs : %d",pipe->regID_EX_atual.rs);
    mvprintw(5, (colunaspainel/5 * 2) - 30,"rt : %d",pipe->regID_EX_atual.rt);
    mvprintw(6, (colunaspainel/5 * 2) - 30,"rd : %d",pipe->regID_EX_atual.rd);
    mvprintw(7, (colunaspainel/5 * 2) - 30,"A  : %d",pipe->regID_EX_atual.A);
    mvprintw(8, (colunaspainel/5 * 2) - 30,"B  : %d",pipe->regID_EX_atual.B);
    mvprintw(9, (colunaspainel/5 * 2) - 30,"Imm: %d",pipe->regID_EX_atual.imm);
    mvprintw(10, (colunaspainel/5 * 2) - 30,"Fun: %d",pipe->regID_EX_atual.funct);
    if(memoria != NULL && (pc-1) >= 0) mvprintw(linhaspainel/3 - 1, (colunaspainel/5) * 2 - 30,"ASM: %s",imprimeInstrucao(memoria, pc-2));
    attroff(A_BOLD | COLOR_PAIR(3));

    attron(A_BOLD | COLOR_PAIR(4)); // EX
    mvprintw(1, (colunaspainel/5 * 3) - colunaspainel/10 - 3, "  EX  ");
    mvprintw(3, (colunaspainel/5 * 3) - 30,"ULA : %d",pipe->regEX_MEM_atual.ulaSaida);
    mvprintw(4, (colunaspainel/5 * 3) - 30,"A   : %d",pipe->regEX_MEM_atual.A);
    mvprintw(5, (colunaspainel/5 * 3) - 30,"B   : %d",pipe->regEX_MEM_atual.B);
    mvprintw(6, (colunaspainel/5 * 3) - 30,"RD  : %d",pipe->regEX_MEM_atual.rd);
    mvprintw(7, (colunaspainel/5 * 3) - 30,"ALU : %s",nomeULA(pipe->regEX_MEM_atual.sinais.ulaOp));
    mvprintw(8, (colunaspainel/5 * 3) - 30,"Branch : %d",pipe->regEX_MEM_atual.sinais.branch);
    mvprintw(9, (colunaspainel/5 * 3) - 30,"Jump   : %d",pipe->regEX_MEM_atual.sinais.jump);
    if(memoria != NULL && (pc-2) >= 0) mvprintw(linhaspainel/3 - 1, (colunaspainel/5 * 3) - 30,"ASM: %s",imprimeInstrucao(memoria, pc-3));
    attroff(A_BOLD | COLOR_PAIR(4));

    attron(A_BOLD | COLOR_PAIR(5)); // MEM
    mvprintw(1, (colunaspainel/5 * 4) - colunaspainel/10 - 4, "  MEM  ");
    mvprintw(3, (colunaspainel/5 * 4) - 30,"ULA : %d",pipe->regMEM_WB_novo.ulaSaida);
    mvprintw(4, (colunaspainel/5 * 4) - 30,"MEM : %d",pipe->regMEM_WB_novo.mem);
    mvprintw(5, (colunaspainel/5 * 4) - 30,"RD  : %d",pipe->regMEM_WB_novo.rd);
    mvprintw(6, (colunaspainel/5 * 4) - 30,"EscMem : %d", pipe->regMEM_WB_novo.sinais.EscMem);
    mvprintw(7, (colunaspainel/5 * 4) - 30,"MemReg : %d", pipe->regMEM_WB_novo.sinais.MemParaReg);
    if(memoria != NULL && (pc-3) >= 0) mvprintw(linhaspainel/3 - 1, (colunaspainel/5 * 4) - 30,"ASM: %s",imprimeInstrucao(memoria, pc-4));
    attroff(A_BOLD | COLOR_PAIR(5));

    attron(A_BOLD | COLOR_PAIR(6)); // WB
    mvprintw(1, (colunaspainel/5 * 5) - colunaspainel/10 - 4, "  WB  ");
    mvprintw(3, (colunaspainel/5 * 5) - 30,"Destino : $%d", pipe->regMEM_WB_atual.rd);
    mvprintw(4, (colunaspainel/5 * 5) - 30,"Valor   : %d", pipe->regMEM_WB_atual.sinais.MemParaReg ? pipe->regMEM_WB_atual.mem : pipe->regMEM_WB_atual.ulaSaida);
    mvprintw(5, (colunaspainel/5 * 5) - 30,"EscReg : %d",pipe->regMEM_WB_atual.sinais.EscReg);
    if(memoria != NULL && (pc-4) >= 0) mvprintw(linhaspainel/3 - 1, (colunaspainel/5 * 5) - 30,"ASM: %s",imprimeInstrucao(memoria, pc-5));
    attroff(A_BOLD | COLOR_PAIR(6));

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
        if(memoria != NULL) {
            mvprintw(linhaVisualInicial + idxPrint, 4, "Ciclo %-2d |  Instrução: %-25s",  linhasLog[i]->st.estat.ciclos, imprimeInstrucao(memoria, pcSalvo));
        }
        idxPrint++;
    }

    attron(A_BOLD | COLOR_PAIR(2));
    mvprintw(linhaspainel/3 + 2, colunaspainel - 30, "BANCO DE REGISTRADORES");
    attroff(A_BOLD | COLOR_PAIR(2));
    
    for(int i = 0; i < 8; i++) {
        mvprintw((linhaspainel/3 + 4) + i, colunaspainel - 23, "$%d: %d", i, bReg[i]);
    }

    attron(A_BOLD | COLOR_PAIR(3));
    mvprintw(linhaspainel/3 + 2, 61, "ESTATÍSTICAS");
    attroff(A_BOLD | COLOR_PAIR(3));

    mvprintw((linhaspainel/3 + 4) + 0, 58,"Ciclos : %d",estatInst->ciclos);
    mvprintw((linhaspainel/3 + 4) + 1, 58,"Instr  : %d",estatInst->total);
    mvprintw((linhaspainel/3 + 4) + 2, 58,"Stalls : %d",estatInst->stalls);
    mvprintw((linhaspainel/3 + 4) + 3, 58,"CPI    : %.2f",estatInst->CPI);
    mvprintw((linhaspainel/3 + 4) + 4, 58,"Instruções por Tipo :");
    mvprintw((linhaspainel/3 + 4) + 5, 58,"Total Tipo R : %d | ADD: %d | SUB: %d | AND: %d | OR: %d",estatInst->tipoR, estatInst->add, estatInst->sub, estatInst->and, estatInst->or);
    mvprintw((linhaspainel/3 + 4) + 6, 58,"Total Tipo I : %d | LOAD: %d | STORE: %d | ADDI: %d | BEQ: %d",estatInst->tipoI, estatInst->lw, estatInst->sw, estatInst->addi, estatInst->beq);
    mvprintw((linhaspainel/3 + 4) + 7, 58,"Total Tipo J : %d | JUMP: %d",estatInst->tipoJ, estatInst->j);

    attron(A_BOLD | COLOR_PAIR(4));
    mvprintw(linhaspainel - 3, 3, " OPÇÕES DE EXECUÇÃO ");
    attroff(A_BOLD | COLOR_PAIR(4));
    
    for(int i = 0; i < 3; i++) {
        if(selecionado == i) attron(A_REVERSE | A_BOLD);
        mvprintw(linhaspainel - 2, 3 + (i * 18), " %s ", opcoesExec[i]);
        if(selecionado == i) attroff(A_REVERSE | A_BOLD);
    }

    attron(A_BOLD | COLOR_PAIR(4));
    mvprintw((linhaspainel/3)*2 + 1, (colunaspainel/5) * 4 + 15, " MENU PRINCIPAL ");
    attroff(A_BOLD | COLOR_PAIR(4));

    for(int i = 0; i < 6; i++) {
        if(selecionado == i + 3) attron(A_REVERSE | A_BOLD);
        mvprintw((linhaspainel/3)*2 + 5 + i, (colunaspainel/5) * 4 + 10, " %-20s ", opcoesMenu[i]);
        if(selecionado == i + 3) attroff(A_REVERSE | A_BOLD);
    }

    refresh();
}