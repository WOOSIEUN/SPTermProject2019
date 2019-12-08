LIBS = -lcurses
CC = gcc

moyeora :main.o
	${CC} -o moyeora main.o  ${LIBS}

main.o: MOYEORA_main.c
	${CC} -o main.o MOYEORA_main.c MOYEORA_header.h ${LIBS}


