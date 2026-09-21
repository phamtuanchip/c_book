CC = gcc
CFLAGS = -Wall -Wextra -std=c11

all: main

module.o: module.c module.h
	$(CC) $(CFLAGS) -c module.c -o module.o

main.o: main.c module.h
	$(CC) $(CFLAGS) -c main.c -o main.o

main: main.o module.o
	$(CC) $(CFLAGS) -o main main.o module.o

clean:
	rm -f *.o main
