CC = gcc
CFLAGS = -Wall -g -std=gnu99 -lrt -pthread
LIBRARY = libinfo.h

all: slave master vista

slave: slave.c $(LIBRARY)
	$(CC) $(CFLAGS) slave.c -o slave

master: master.c $(LIBRARY)
	$(CC) $(CFLAGS) master.c -o master

vista: vista.c $(LIBRARY)
	$(CC) $(CFLAGS) vista.c -o vista

clean:
	rm -rf slave master vista resultFile

.PHONY:	all clean
	