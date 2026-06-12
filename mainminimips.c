#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "minimips.h"
#include "hazards.h"

void inicializa_cores() {
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);    // Títulos e Bordas
    init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Mensagens de Sucesso / Regs
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Alertas / Estatísticas
    init_pair(4, COLOR_RED, COLOR_BLACK);     // Stalls e NOPs
}

int main() {
    // Inicialização mandatória do ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    inicializa_cores();

    // Declarações locais de dados do processador
    int *bReg = inicializaBReg();
    int *memDados = inicializaMemDados();
    instrucao *memoria = NULL;
    int pc = 0;

    registradoresPipeline pipe;
    memset(&pipe, 0, sizeof(registradoresPipeline));

    estatInstrucoes estatInst;
    memset(&estatInst, 0, sizeof(estatInstrucoes));

    historico hist;
    hist.topo = 0;

    // Carrega dados iniciais simulados
    int linhas = contaLinhas("instrucoes.txt");
    lerMem("instrucoes.txt", &memoria, linhas);
    lerMemDados("memdados.txt", &memDados);

    int opcao = 0;

    while (opcao != 8) {
        clear();

        // Cabeçalho da Interface Principal
        attron(A_BOLD | COLOR_PAIR(1));
        mvprintw(1, 5, "====== MINI-MIPS PIPELINE SIMULATOR ======");
        attroff(A_BOLD | COLOR_PAIR(1));

        mvprintw(3, 5, "PC Atual: %d", pc);
        mvprintw(5, 5, "1. Executar instrucao (Step Pipeline)");
        mvprintw(6, 5, "2. Executar programa completo (Run)");
        mvprintw(7, 5, "3. Voltar uma instrucao (Undo)");
        mvprintw(8, 5, "4. Visualizar Memorias");
        mvprintw(9, 5, "5. Exibir Estatisticas");
        mvprintw(10, 5, "6. Salvar em ASM");
        mvprintw(11, 5, "7. Salvar Estado da Memoria (.dat)");
        mvprintw(12, 5, "8. Sair");

        // Imprime o banco de registradores na lateral direita
        imprimeBancoRegistradores(bReg);

        mvprintw(15, 5, "Escolha uma opcao: ");
        refresh();

        // Leitura tratada com ncurses
        echo();
        scanw("%d", &opcao);
        noecho();

        switch (opcao) {
            case 1:
                salvaEstado(&hist, pc, memDados, bReg, &estatInst);
                step_pipeline(memoria, bReg, &pc, memDados, &pipe, &estatInst);
                break;

            case 2:
                run_pipeline(memoria, bReg, &pc, memDados, &pipe, &estatInst);
                break;

            case 3:
                voltaInstrucao(&hist, &pc, memDados, bReg, &estatInst);
                break;

            case 4:
                imprimeMemorias(memoria, memDados);
                break;

            case 5:
                imprimeEstatistica(estatInst);
                break;

            case 6:
                salvaASM(memoria, linhas);
                break;

            case 7:
                salvaDAT(memDados);
                break;

            case 8:
                break;

            default:
                attron(COLOR_PAIR(4));
                mvprintw(17, 5, "Opcao invalida! Pressione qualquer tecla...");
                attroff(COLOR_PAIR(4));
                getch();
                break;
        }
    }

    // Liberação de Memória e Fechamento Seguro
    free(bReg);
    free(memDados);
    free(memoria);

    endwin();
    return 0;
}
