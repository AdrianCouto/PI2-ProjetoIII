
help:
	@echo "Funcionalidades:"
	@echo "Digite make compile ou apenas make para compilar o arquivo;"
	@echo "Digite make run para compilar e executar o arquivo;"
	@echo "Digite make rm para excluir os arquivos gerados."

compile:
	gcc -c -Wall -Wextra -g funcoes.c -o funcoes.o
	gcc -Wall -Wextra -g mainminimips.c funcoes.o -lncurses -o exec

run:
	gcc -c -Wall -Wextra -g funcoes.c -o funcoes.o
	gcc -Wall -Wextra -g mainminimips.c funcoes.o -lncurses -o exec
	./exec

rm:
	rm -f funcoes.o
	rm -f exec
	rm -f *.asm
