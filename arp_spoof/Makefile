CC=gcc
CFLAGS=-Wall -g

all: arpspoof

arpspoof: arpspoof.c
	$(CC) -o arpspoof arpspoof.c  $(CFLAGS)

clean:
	$(RM) arpspoof *.o
	$(RM) -rf *.dSYM *~
