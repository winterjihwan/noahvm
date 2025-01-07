CFLAGS=-Wall -Wextra -std=c11 -pedantic -Wmissing-prototypes
# -Wswitch-enum

LIB = ./src/lexer.c ./src/compiler.c ./src/vm.c ./src/table.c

main: ./src/main.c ./src/lexer.c ./src/compiler.c ./src/vm.c ./src/table.c
	$(CC) $(CFLAGS) $(LIB) -g -o main ./src/main.c
