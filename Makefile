CC=gcc
CFLAGS=-g -Wall -O0

all: stride tlb

stride: stride.o
	$(CC) $(CFLAGS) -o $@ stride.o

tlb: tlb.o
	$(CC) $(CFLAGS) -o $@ tlb.o

.PHONY: clean
clean:
	rm -f *.o stride tlb

.c.o:
	$(CC) -c $(CFLAGS) $<
