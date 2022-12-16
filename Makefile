CC=gcc
CFLAGS=-Wall


all: player GS

player: interface.o socket.o game.o player.o
	$(CC) $(CFLAGS) -o player bin/interface.o bin/socket.o bin/game.o bin/player.o

GS: interface.o socket.o GS.o
	$(CC) $(CFLAGS) -o GS bin/interface.o bin/socket.o bin/GS.o

interface.o:
	$(CC) $(CFLAGS) -o bin/interface.o -c src/interface.c

socket.o:
	$(CC) $(CFLAGS) -o bin/socket.o -c src/socket.c

game.o:
	$(CC) $(CFLAGS) -o bin/game.o -c src/game.c

player.o:
	$(CC) $(CFLAGS) -o bin/player.o -c src/player.c

GS.o:
	$(CC) $(CFLAGS) -o bin/GS.o -c src/GS.c

clean:
	rm -f bin/interface.o bin/socket.o bin/game.o bin/player.o bin/GS.o player GS