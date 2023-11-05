CC=gcc
CFLAGS=-Wall -g

all: udp_scan

udp_scan: udp_scan.c
	$(CC) -o udp_scan udp_scan.c  $(CFLAGS)

clean:
	$(RM) udp_scan *.o
	$(RM) -rf *.dSYM *~
