CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGETS = reverse_string dynamic_string

all: $(TARGETS)

reverse_string: reverse_string.c
	$(CC) $(CFLAGS) -o reverse_string reverse_string.c

dynamic_string: dynamic_string.c
	$(CC) $(CFLAGS) -o dynamic_string dynamic_string.c

clean:
	rm -f $(TARGETS)
