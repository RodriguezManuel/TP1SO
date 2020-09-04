CC = gcc

CFLAGS = -Wall -g

all: slave

slave: slave.c
	$(CC) $(CFLAGS) slave.c -o slave

clean:
	rm -rf slave

.PHONY:	all clean