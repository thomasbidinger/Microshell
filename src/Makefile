CC=gcc
CFLAGS= -g -c -Wall

ush: ush.o expand.o
	$(CC) $(CFLAGS) ush.c
	$(CC) $(CFLAGS) builtin.c
	$(CC) $(CFLAGS) expand.c
	$(CC) -g -o ush ush.o expand.o builtin.o

clean:
	rm -r ush ush.o expand.o builtin.o

#depends
ush.o expand.o builtin.o: defn.h
