CFLAGS=-Wall -Wextra -std=c11 -pedantic
LIB = jsmn.c

main: main.c 
	$(CC) $(CFLAGS) $(LIB) -o main main.c
