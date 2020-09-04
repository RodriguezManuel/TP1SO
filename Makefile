all: slave

slave:	slave.c
	gcc -Wall -g -std=c99 slave.c -o slave

clean:
	rm -rf slave

.PHONY:	all clean