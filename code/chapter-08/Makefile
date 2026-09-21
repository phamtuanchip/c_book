CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGETS = pointer_examples qsort_cmp

all: $(TARGETS)

pointer_examples: pointer_examples.c
	$(CC) $(CFLAGS) -o pointer_examples pointer_examples.c

qsort_cmp: qsort_cmp.c
	$(CC) $(CFLAGS) -o qsort_cmp qsort_cmp.c

clean:
	rm -f $(TARGETS)
