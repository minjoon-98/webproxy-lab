CC = gcc
CFLAGS = -O2 -Wall -I .
LIB = -lpthread

all: echoclient echoserverp echoservers echoservert

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

echoclient: echoclient.c csapp.o
	$(CC) $(CFLAGS) -o echoclient echoclient.c csapp.o $(LIB)

echoserverp: echoserverp.c csapp.o
	$(CC) $(CFLAGS) -o echoserverp echoserverp.c csapp.o $(LIB)

echoservers: echoservers.c csapp.o
	$(CC) $(CFLAGS) -o echoservers echoservers.c csapp.o $(LIB)

echoservert: echoservert.c csapp.o
	$(CC) $(CFLAGS) -o echoservert echoservert.c csapp.o $(LIB)
        
clean:
	rm -f *.o echoclient echoserverp echoservers echoservert *~
