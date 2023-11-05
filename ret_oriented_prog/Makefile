CC=gcc
CFLAGS=-Wall -g

all: rop

rop: rop.c
	$(CC) -o rop rop.c  $(CFLAGS)

clean:
	$(RM) rop *.o
	$(RM) -rf *.dSYM *~
