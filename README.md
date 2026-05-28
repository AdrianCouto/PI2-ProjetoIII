# PI2-ProjetoIII
Serão utilizados como base para continuidade deste, os projetos desenvolvidos anteriormente: https://github.com/gabrieltm-sudo/PI2-ProjetoI e https://github.com/gabrieltm-sudo/PI2-ProjetoII.

Simulador acadêmico de uma arquitetura MiniMIPS de 8 bits, organizado em Pipeline. Implementado em C com interface por menu, suporta operações de carga/armazenamento, instruções aritméticas e de controle de fluxo.

**Autores**

- Alex Maron Vaz — https://github.com/AlexMaronVaz
- Adrian do Couto Martins — https://github.com/AdrianCouto
- Gabriel Torres Machado — https://github.com/gabrieltm-sudo

**Principais funcionalidades**

Simulação de execução pipeline de instruções MiniMIPS
Suporte a leitura/escrita de memória (arquivo de extensão .mem)
Interface interativa do menu para carregar e executar programas

**Estrutura do repositório**

- mainminimips.c — ponto de entrada / interface do simulador
- funcoes.c — implementação das funções do simulador
- minimips.h — definições e protótipos
- memoria.mem — arquivo de memória com instruções e dados
- makefile — regras de compilação e execução
- Arquivos Importantes/instMiniMips.txt — instruções e formato suportado

**Requisitos**

- GCC
- Make
- Ncurse

**Como compilar e executar**

Use as flags do makefile a partir do diretório do projeto:

make compile      # compila os binários
make run          # executa o simulador (./exec)
make compilerun   # compila e executa em sequência
make rm           # remove arquivos gerados
Observações

Projeto para fins educacionais (disciplina Projeto Integrador II).
Consulte Arquivos Importantes/instMiniMips.txt para o formato das instruções a serem carregadas na memória.
