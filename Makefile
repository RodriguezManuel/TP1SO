CC = gcc
CFLAGS = -Wall -g -std=gnu99 -lrt -pthread

all: slave master vista

slave: slave.c
	$(CC) $(CFLAGS) slave.c -o slave

master: master.c
	$(CC) $(CFLAGS) master.c -o master

vista: vista.c
	$(CC) $(CFLAGS) vista.c -o vista

clean:
	rm -rf slave master vista resultFile

.PHONY:	all clean