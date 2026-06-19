
help:
	@echo "Funcionalidades:"
	@echo "Digite make compile ou apenas make para compilar o arquivo;"
	@echo "Digite make run para compilar e executar o arquivo;"
	@echo "Digite make rm para excluir os arquivos gerados."

compile:
	gcc -c -Wall -Wextra -g funcoes.c -o funcoes.o
	gcc -c -Wall -Wextra -g hazards.c -o hazards.o
	gcc -Wall -Wextra -g mainminimips.c funcoes.o hazards.o -lncursesw -o exec

run:
	gcc -c -Wall -Wextra -g funcoes.c -o funcoes.o
	gcc -c -Wall -Wextra -g hazards.c -o hazards.o
	gcc -Wall -Wextra -g mainminimips.c funcoes.o hazards.o -lncursesw -o exec
	./exec

rm:
	rm -f *.o
	rm -f exec
	rm -f *.asm
