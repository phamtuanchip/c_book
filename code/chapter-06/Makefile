CC = gcc
CFLAGS = -Wall -Wextra -std=c11

all: demo_math

math.o: math.c math.h
	$(CC) $(CFLAGS) -c math.c -o math.o

main.o: main.c math.h
	$(CC) $(CFLAGS) -c main.c -o main.o

demo_math: main.o math.o
	$(CC) $(CFLAGS) -o demo_math main.o math.o

clean:
	rm -f *.o demo_math
