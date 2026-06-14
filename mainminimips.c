#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ncurses.h>
#include "minimips.h"


static void iniciaNcurses(void){
    initscr();

    start_color();
    use_default_colors();

    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, COLOR_YELLOW, -1);
    init_pair(3, COLOR_GREEN, -1);
    init_pair(4, COLOR_RED, -1);

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
}



int main(){

    iniciaNcurses();

    mousemask(0, NULL);

    int pc = 0, opcao, linhas = 0;
    estatInstrucoes estatInst = {0};

    instrucao *memoria = NULL;

    historico hist;
    hist.topo = 0;

    int *bReg = inicializaBReg();
    int *memDados = inicializaMemDados();

    registradoresPipeline pipe = {0};
    //inicializa_pipeline(&pipe);

    int selecionado = 0;
    int tecla;

    char *opcoes[] = {
        "Carregar Memoria de Instrucoes (.mem)",
        "Carregar Memoria de Dados (.dat)",
        "Imprimir memorias (instrucoes e dados)",
        "Imprimir Banco de Registradores",
        "Imprimir todo o Simulador",
        "Salvar .asm",
        "Salvar .dat",
        "Executa programa (run)",
        "Executa uma instrucao (step)",
        "Volta uma instrucao (back)",
        "Sair do Programa"
    };

    const int totalOpcoes = 11;
 
    int linhaspainel, colunaspainel;

    while (1) {

    getmaxyx(stdscr, linhaspainel, colunaspainel);

    clear();

    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(2, (colunaspainel/2)-11, "+-------------------+");
    mvprintw(3, (colunaspainel/2)-11, "| MiniMIPS Pipeline |");
    mvprintw(4, (colunaspainel/2)-11, "+-------------------+");
    attroff(COLOR_PAIR(3) | A_BOLD);
    
    printBorda(linhaspainel, colunaspainel);

    for(int k = 0; k < totalOpcoes; k++) {
        if(k == selecionado){
            attron(A_REVERSE | A_BOLD);
    }
        mvprintw(9 + k, colunaspainel/3, "%s", opcoes[k]);
    
        if(k == selecionado){
            attroff(A_REVERSE | A_BOLD);
        }
    }
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(22, (colunaspainel/3), "Use as setas e ENTER");
    attroff(COLOR_PAIR(4) | A_BOLD);

    refresh();

    tecla = getch();

    switch(tecla) {

        case KEY_UP:
            selecionado--;
            if(selecionado < 0){
                selecionado = totalOpcoes - 1;
            }
            continue;

        case KEY_DOWN:
            selecionado++;
            if(selecionado >= totalOpcoes){
                selecionado = 0;
            }
            continue;

        case 10:
            opcao = selecionado;
            break;

        default:
            continue;
    }

    switch(opcao) {

        case 0:

            char arq[20];
            
            clear();
            
            printBorda(linhaspainel, colunaspainel);
            
            mvprintw((linhaspainel/2), (colunaspainel/2) - 20, "Digite o nome do arquivo .mem: ");

            echo();
            curs_set(1);
            mvgetnstr((linhaspainel/2), (colunaspainel/2) + 11, arq, sizeof(arq)-1);
            noecho();
            curs_set(0);

            linhas = contaLinhas(arq);
            
            if (lerMem(colunaspainel, linhaspainel, arq, &memoria, linhas)) {

            clear();

            printBorda(linhaspainel, colunaspainel);

            mvprintw((linhaspainel/2), (colunaspainel/2) - 10, "Arquivo carregado!");
                } else {
                clear();
                printBorda(linhaspainel, colunaspainel);
                mvprintw((linhaspainel/2), (colunaspainel/2) - 12, "Falha ao carregar arquivo!");
            }

            refresh();
            getch();
            break;

        case 1:

            char arqMem[20];

            clear();
            
            printBorda(linhaspainel, colunaspainel);
            
            mvprintw((linhaspainel/2), (colunaspainel/2) - 20, "Digite o nome do arquivo .dat: ");

            echo();
            curs_set(1);
            mvgetnstr((linhaspainel/2), (colunaspainel/2) + 11, arqMem, sizeof(arqMem)-1);
            noecho();
            curs_set(0);

            if(lerMemDados(colunaspainel, linhaspainel, arqMem, &memDados)){
                clear();
                printBorda(linhaspainel, colunaspainel);
                mvprintw((linhaspainel/2), (colunaspainel/2) - 10, "Arquivo carregado!");

            }else {
                clear();
                printBorda(linhaspainel, colunaspainel);
                mvprintw((linhaspainel/2), (colunaspainel/2) - 12, "Falha ao carregar arquivo!");
            }

            refresh();
            getch();

            break;

        case 2:
            endwin();
            imprimeMemorias(colunaspainel, linhaspainel, memoria, memDados);
            iniciaNcurses();
            break;

        case 3:
            endwin();
            imprimeBancoRegistradores(bReg);
            iniciaNcurses();
            break;

        case 4:
            //Imprimir simulador
            printf("\nImpressão do simulador:\n");
            do{
                printf("\n1. Banco de Registradores\n2. Memórias (Dados ou Instruções)\n3. Estatísticas das Instruções)\n");
                printf("\nSelecione uma das opções acima: ");
                scanf("%d", &opcao);
                switch(opcao){
                    case 1:
                        imprimeBancoRegistradores(bReg);
                        break;
                    case 2:
                        imprimeMemorias(colunaspainel, linhaspainel, memoria, memDados);
                        break;
                    case 3:
                        imprimeEstatistica(estatInst);
                        break;
                    default:
                        printf("Opção inválida! Por favor selecione uma das opções disponíveis.\n");
                        break;
                    }
                }while(opcao<1 || opcao>3);
                break;
        case 5:
            endwin();
            salvaASM(colunaspainel, linhaspainel, memoria, linhas);
            iniciaNcurses();
            break;

        case 6:
            endwin();
            salvaDAT(memDados);
            iniciaNcurses();
            break;

        case 7:
            endwin();
            run_pipeline(memoria, bReg, &pc, memDados, &pipe, &estatInst);
            iniciaNcurses();
            break;

        case 8:
            endwin();
            salvaEstado(&hist, pc, memDados, bReg, &estatInst);
            step_pipeline(memoria, bReg, &pc, memDados, &pipe, &estatInst);
            iniciaNcurses();
            break;

        case 9:
            endwin();
            voltaInstrucao(&hist, &pc, memDados, bReg, &estatInst);
            iniciaNcurses();
            break;

        case 10:
            endwin();
            free(bReg);
            free(memoria);
            free(memDados);
            printf("\nSaindo do programa...\n");
            return 0;
        }
    }
        return 0;
}