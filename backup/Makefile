CC = gcc
TARGET = TCPserver TCPclient

all: $(TARGET)

TCPserver : TCPserver.o
	$(CC) -o TCPserver TCPserver.o

TCPclient : TCPclient.o
	$(CC) -o TCPclient TCPclient.o

TCPserver.o : TCPserver.c headers.h
	$(CC) -c -o TCPserver.o TCPserver.c

TCPclient.o : TCPclient.c headers.h
	$(CC) -c -o TCPclient.o TCPclient.c 

clean :
	rm TCPserver.o TCPserver
	rm TCPclient.o TCPclient


