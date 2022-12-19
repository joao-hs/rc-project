CC=gcc
CFLAGS=-Wall


all: player GS

player: socket.o game.o interface.o player.o
	$(CC) $(CFLAGS) -o player bin/socket.o bin/game.o bin/interface.o bin/player.o

GS: socket.o interface.o game.o GS.o
	$(CC) $(CFLAGS) -o GS bin/socket.o bin/game.o bin/interface.o bin/GS.o

socket.o:
	$(CC) $(CFLAGS) -o bin/socket.o -c src/socket.c

game.o:
	$(CC) $(CFLAGS) -o bin/game.o -c src/game.c

interface.o:
	$(CC) $(CFLAGS) -o bin/interface.o -c src/interface.c

player.o:
	$(CC) $(CFLAGS) -o bin/player.o -c src/player.c

GS.o:
	$(CC) $(CFLAGS) -o bin/GS.o -c src/GS.c

clean:
	rm -f bin/interface.o bin/socket.o bin/game.o bin/player.o bin/GS.o player GS