CFLAGS=-Wall -Wextra -std=c11 -pedantic
LIB = lexer.c

main: main.c lexer.c
	$(CC) $(CFLAGS) $(LIB) -g -o main main.c
