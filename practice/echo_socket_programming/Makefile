CC = gcc
CFLAGS = -O2 -Wall -I .
LIB = -lpthread

all: echoclient echoserveri

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

echoclient: echoclient.c csapp.o
	$(CC) $(CFLAGS) -o echoclient echoclient.c csapp.o $(LIB)

echoserveri: echoserveri.c csapp.o
	$(CC) $(CFLAGS) -o echoserveri echoserveri.c csapp.o $(LIB)
        
clean:
	rm -f *.o echoclient echoserveri *~

	

# # echoclient만 파일명에 맞게 바꿔주면 됨.
# TARGET = echoclient
# OBJS = 	echoclient.o csapp.o
# %.o: %.c
# 	gcc -c $<
# $(TARGET) : $(OBJS)
# 	gcc -o $(TARGET) $(OBJS)
# clean:
# 	rm -f *.o
# 	rm -f $(TARGET)