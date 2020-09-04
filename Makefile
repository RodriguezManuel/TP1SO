CC = gcc

CFLAGS = -Wall -g -std=gnu99

all: slave master

slave: slave.c
	$(CC) $(CFLAGS) slave.c -o slave

master: master.c
	$(CC) $(CFLAGS) master.c -o master

clean:
	rm -rf slave master

.PHONY:	all clean