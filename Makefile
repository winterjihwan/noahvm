CFLAGS=-Wall -Wextra -std=c11 -pedantic
LIB =

main: main.c 
	$(CC) $(CFLAGS) $(LIB) -o main main.c
