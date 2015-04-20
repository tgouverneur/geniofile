CC=gcc

all:		genfileio

genfileio:	genfileio.c
		$(CC) -o genfileio genfileio.c

clean:
		rm -f genfileio
