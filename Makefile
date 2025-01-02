CFLAGS=-Wall -Wextra -std=c11 -pedantic -Wmissing-prototypes
# -Wswitch-enum
LIB = lexer.c compiler.c vm.c table.c

main: main.c lexer.c compiler.c vm.c table.c
	$(CC) $(CFLAGS) $(LIB) -g -o main main.c

vm: vm.c
	$(CC) $(CFLAGS) -g -o vm vm.c

table: table.c
	$(CC) $(CFLAGS) -g -o table table.c
