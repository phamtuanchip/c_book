CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDLIBS = -lm

all: prime_check multiplication_table

prime_check: prime_check.c
	$(CC) $(CFLAGS) -o prime_check prime_check.c $(LDLIBS)

multiplication_table: multiplication_table.c
	$(CC) $(CFLAGS) -o multiplication_table multiplication_table.c

clean:
	rm -f prime_check multiplication_table
