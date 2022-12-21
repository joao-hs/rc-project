CC=gcc
CFLAGS=-Wall


all: player GS

debug: CFLAGS += -g
debug: all

player: socket.o session.o games.o interface.o player.o
	$(CC) $(CFLAGS) -o player bin/socket.o bin/session.o bin/games.o bin/interface.o bin/player.o

GS: socket.o session.o games.o interface.o GS.o
	$(CC) $(CFLAGS) -o GS bin/socket.o bin/session.o bin/games.o bin/interface.o bin/GS.o

socket.o:
	$(CC) $(CFLAGS) -o bin/socket.o -c src/socket.c

session.o:
	$(CC) $(CFLAGS) -o bin/session.o -c src/session.c

games.o:
	$(CC) $(CFLAGS) -Wformat-truncation=0 -o bin/games.o -c src/games.c

interface.o:
	$(CC) $(CFLAGS) -o bin/interface.o -c src/interface.c

player.o:
	$(CC) $(CFLAGS) -o bin/player.o -c src/player.c

GS.o:
	$(CC) $(CFLAGS) -o bin/GS.o -c src/GS.c

clean:
	rm -f bin/interface.o bin/socket.o bin/session.o bin/player.o bin/GS.o player GS