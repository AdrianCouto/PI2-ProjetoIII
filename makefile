compile:
	gcc -c -g funcoes.c -o funcoes
	gcc -Wall -Wextra -g mainminimips.c funcoes -o exec

help:
	@echo "Funcionalidades:"
	@echo "Digite make compile ou apenas make para compilar o arquivo;"
	@echo "Digite make run para compilar e executar o arquivo;"
	@echo "Digite make rm para excluir os arquivos gerados."

run:
	gcc -c -g funcoes.c -o funcoes
	gcc -Wall -Wextra -g mainminimips.c funcoes -o exec
	./exec

rm:
	rm -f funcoes
	rm -f exec
	rm -f *.asm
