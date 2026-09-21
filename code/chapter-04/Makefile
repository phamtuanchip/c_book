CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGETS = macro_min min_inline

all: $(TARGETS)

macro_min: macro_min.c
	$(CC) $(CFLAGS) -o macro_min macro_min.c

min_inline: min_inline.c
	$(CC) $(CFLAGS) -o min_inline min_inline.c

clean:
	rm -f $(TARGETS)
