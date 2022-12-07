CC=gcc
CFLAGS=-Wall


all: player #GS

player: interface.o player.o
	$(CC) $(CFLAGS) -o player bin/src/interface.o bin/src/player.o

GS: interface.o GS.o
	$(CC) $(CFLAGS) -o GS bin/src/interface.o bin/src/GS.o

interface.o:
	$(CC) $(CFLAGS) -o bin/src/interface.o -c src/interface.c

player.o:
	$(CC) $(CFLAGS) -o bin/src/player.o -c src/player.c

GS.o:
	$(CC) $(CFLAGS) -o bin/src/GS.o -c src/GS.c

clean:
	rm -f bin/src/interface.o bin/src/player.o bin/src/GS.o player GS