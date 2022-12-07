CC=gcc
CFLAGS=-Wall


all: player #GS

player: interface.o player.o
	$(CC) $(CFLAGS) -o player interface.o player.o

#GS: interface.o GS.o
#	$(CC) $(CFLAGS) -o GS interface.o GS.o

clean:
	rm -f interface.o player.o GS.o player GS