#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ncurses.h>
#include "minimips.h"


/*static void iniciaNcurses(void){
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
}*/



int main(){

    // iniciaNcurses();

    int pc = 0, opcao, linhas = 0;
    estatInstrucoes estatInst = {0};

    instrucao *memoria = NULL;

    historico hist;
    hist.topo = 0;

    int *bReg = inicializaBReg();
    int *memDados = inicializaMemDados();

    registradoresPipeline pipe = {0};
    // inicializa_pipeline(&pipe);

    while (1) {
        
        printf("\n=============== MiniMIPS Pipeline ===============\n\n");
        printf("0. Sair do Programa\n");
        printf("1. Carregar Memória de Instruções (.mem)\n");
        printf("2. Carregar Memória de Dados (.dat)\n");
        printf("3. Imprimir memórias (instruções e dados)\n");
        printf("4. Imprimir Banco de Registradores\n");
        printf("5. Imprimir todo o Simulador\n");
        printf("6. Salvar .asm\n");
        printf("7. Salvar .dat\n");
        printf("8. Executa programa (run)\n");
        printf("9. Executa uma instrução (step)\n");
        printf("10. Volta uma instrução (back)\n\n");
        printf("Digite uma opção: ");
        scanf("%d", &opcao);
        getchar();

        printf("====================================================");

        switch (opcao) {
            case 1:
                //Carregar Mem de Instr.
                char arq[20];
                printf("\nDigite o nome do arquivo da memória de instruções (.mem): ");

                fgets(arq, sizeof(arq), stdin);
                arq[strcspn(arq, "\n")] = '\0';

                linhas = contaLinhas(arq);
                printf("\n%d Instruções carregadas!\n", linhas);

                lerMem(arq, &memoria, linhas);
                break;

            case 2:
                //Carregar Mem de Dados
                char arqMem[20];
                printf("\nDigite o nome do arquivo da memória de dados (.dat): ");

                fgets(arqMem, sizeof(arqMem), stdin);
                arqMem[strcspn(arqMem, "\n")] = '\0';

                lerMemDados(arqMem, &memDados);

                break;

            case 3:
                // Imprimir memórias (tanto instruções quando dados)
                imprimeMemorias(memoria,memDados);

                break;

            case 4:
                //Imprimir Banco de Registradores
                imprimeBancoRegistradores(bReg);
                break;
            case 5:
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
                            imprimeMemorias(memoria,memDados);
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

            case 6:
                // Salvar .asm
                salvaASM(memoria, linhas);
                break;

            case 7:
                // Salvar .dat
                salvaDAT(memDados);
                break;

            case 8:
                //Executar programa (run)
                run_pipeline(memoria, bReg, &pc, memDados, &pipe, &estatInst);
                break;

            case 9:
                //Executa instrução (step)
                salvaEstado(&hist, pc, memDados, bReg, &estatInst);
                step_pipeline(memoria, bReg, &pc, memDados, &pipe, &estatInst);
                break;

            case 10:
                //Voltar instrução (back)
                voltaInstrucao(&hist, &pc, memDados, bReg, &estatInst);
                break;

            case 0:
                //Sair
                //endwin();
                free(bReg);
                free(memoria);
                free(memDados);
                printf("\nSaindo do programa...\n");
                return 0;

            default:
                printf("\nOpção inválida!\n");
                break;
        }
    }
        
/*  Menu em Ncurse
        clear();
        attron(A_BOLD);
        mvprintw(1, 5, "=============== MiniMIPS Pipeline ===============");
        attroff(A_BOLD);

        attron(A_BOLD);
        mvprintw(3, 5,  "0. Sair do Programa");
        mvprintw(4, 5,  "1. Carregar Memoria de Instrucoes (.mem)");
        mvprintw(5, 5,  "2. Carregar Memoria de Dados (.dat)");
        mvprintw(6, 5,  "3. Imprimir memorias (instrucoes e dados)");
        mvprintw(7, 5,  "4. Imprimir Banco de Registradores");
        mvprintw(8, 5,  "5. Imprimir todo o Simulador");
        mvprintw(9, 5,  "6. Salvar .asm");
        mvprintw(10, 5, "7. Salvar .dat");
        mvprintw(11, 5, "8. Executa programa (run)");
        mvprintw(12, 5, "9. Executa uma instrucao (step)");
        mvprintw(13, 5, "10. Volta uma instrucao (back)");
        attroff(A_BOLD);

        attron(A_BOLD);
        mvprintw(15, 5, "Selecione uma das opções acima: ");
        echo();
        curs_set(1);
        mvscanw(15, 39, "%d", &opcao);
        noecho();
        curs_set(0);
        
        mvprintw(18, 5, "=================================================");
        attroff(A_BOLD);


        switch (opcao) {
            case 1:
                endwin();
                //Carregar Mem de Instr.
                char arq[20];
                printf("\nDigite o nome do arquivo da memória de instruções (.mem): ");

                fgets(arq, sizeof(arq), stdin);
                arq[strcspn(arq, "\n")] = '\0';

                linhas = contaLinhas(arq);
                printf("\n%d Instruções carregadas!\n", linhas);

                lerMem(arq, &memoria, linhas);
                iniciaNcurses();
                break;

            case 2:
                endwin();
                //Carregar Mem de Dados
                char arqMem[20];
                printf("\nDigite o nome do arquivo da memória de dados (.dat): ");

                fgets(arqMem, sizeof(arqMem), stdin);
                arqMem[strcspn(arqMem, "\n")] = '\0';

                lerMemDados(arqMem, &memDados);

                iniciaNcurses();

                break;

            case 3:
                endwin();
                // Imprimir memórias (tanto instruções quando dados)
                imprimeMemorias(memoria,memDados);

                iniciaNcurses();

                break;

            case 4:
                endwin();
                //Imprimir Banco de Registradores
                imprimeBancoRegistradores(bReg);

                iniciaNcurses();
                break;
            case 5:
                clear();
                do{

                    attron(A_BOLD);
                    mvprintw(1, 5, "======= Impressao do simulador =======");
                    attroff(A_BOLD);

                    attron(A_BOLD);
                    mvprintw(3, 5, "1. Banco de Registradores");
                    mvprintw(4, 5, "2. Memorias (Dados ou Instrucoes)");
                    mvprintw(5, 5, "3. Estatisticas das Instrucoes");
                attroff(A_BOLD);
                
                attron(A_BOLD);
                mvprintw(7, 5, "Selecione uma das opções acima: ");
                attroff(A_BOLD);
                refresh();

                echo();
                curs_set(1);
                mvscanw(7, 39, "%d", &opcao);
                noecho();
                curs_set(0);
                mvprintw(6, 5, "========================================");
                
                clear();
                switch(opcao){
                    case 1:
                        endwin();
                        imprimeBancoRegistradores(bReg);
                        break;
                    case 2:
                        endwin();
                        imprimeMemorias(memoria, memDados);
                        break;
                    case 3:
                        imprimeEstatistica( estatInst);
                        break;
                    default:
                        mvprintw(9, 5, "Opção inválida! Por favor, selecione uma das opções disponíveis.");
                        refresh();
                }
            }while(opcao<1 || opcao>3);
                
                iniciaNcurses();
                break;

            case 6:
                endwin();
                // Salvar .asm
                salvaASM(memoria, linhas);
                iniciaNcurses();
                break;

            case 7:
                endwin();
                // Salvar .dat
                salvaDAT(memDados);
                iniciaNcurses();
                break;

            case 8:
                endwin();
                //Executar programa (run)
                run_pipeline(memoria, bReg, &pc, memDados, &pipe, &estatInst);
                iniciaNcurses();
                break;

            case 9:
                endwin();
                //Executa instrução (step)
                salvaEstado(&hist, pc, memDados, bReg, &estatInst);
                step_pipeline(memoria, bReg, &pc, memDados, &pipe, &estatInst);
                iniciaNcurses();
                break;

            case 10:
                endwin();
                //Voltar instrução (back)
                voltaInstrucao(&hist, &pc, memDados, bReg, &estatInst);
                iniciaNcurses();
                break;

            case 0:
                //Sair
                endwin();
                free(bReg);
                free(memoria);
                free(memDados);
                printf("\nSaindo do programa...\n");
                return 0;

            default:
                attron(COLOR_PAIR(4) | A_BOLD);
                mvprintw(18, 5, "Opcao invalida! Pressione qualquer tecla.");
                attroff(COLOR_PAIR(4) | A_BOLD);

                refresh();
                getch();
                break;
        }
    }
*/
    return 0;
}
