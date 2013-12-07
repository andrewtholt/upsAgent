
BINS=upsAgent
CC=g++
CFLAGS=-g

all:	$(BINS)

upsAgent:	upsAgent.cpp ups.h ups.o
	$(CC) $(CFLAGS) upsAgent.cpp ups.o -o upsAgent

ups.o:	ups.cpp ups.h
	$(CC) -c $(CFLAGS) ups.cpp -o ups.o

install:	all
	cp $(BINS) /usr/local/bin

clean:
	rm -f $(BINS) *.o *~ cscope.out
