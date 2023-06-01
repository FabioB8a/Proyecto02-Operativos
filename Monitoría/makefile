CC = gcc
CFLAGS = -Wall -Wextra

.PHONY: all clean

all: servidor cliente

servidor: servidor.c
	$(CC) -o servidor servidor.c

cliente: cliente.c
	$(CC) -o cliente cliente.c

clean:
	rm -f servidor cliente