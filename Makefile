CC=gcc
TARGET=webserver

all:
	$(CC) WebServer/webserver.c LRU/LinearProbe.c  LRU/LinkedList.c -o webserver/$(TARGET)

clean:
	rm $(TARGET)