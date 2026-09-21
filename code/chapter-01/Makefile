# Simple Makefile for chapter-01 examples
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

all: hello hello_name arithmetic input_validation

hello: hello.c
	$(CC) $(CFLAGS) -o hello hello.c

hello_name: hello_name.c
	$(CC) $(CFLAGS) -o hello_name hello_name.c

arithmetic: arithmetic.c
	$(CC) $(CFLAGS) -o arithmetic arithmetic.c

input_validation: input_validation.c
	$(CC) $(CFLAGS) -o input_validation input_validation.c

clean:
	rm -f hello hello_name arithmetic input_validation
