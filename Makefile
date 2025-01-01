CFLAGS=-Wall -Wextra -std=c11 -pedantic -Wmissing-prototypes -Wswitch-enum
LIB = lexer.c parser.c

main: main.c lexer.c parser.c
	$(CC) $(CFLAGS) $(LIB) -g -o main main.c

vm: vm.c
	$(CC) $(CFLAGS) -g -o vm vm.c
