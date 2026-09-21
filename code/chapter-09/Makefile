CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGETS = leak_demo use_after_free double_free

all: $(TARGETS)

leak_demo: leak_demo.c
	$(CC) $(CFLAGS) -o leak_demo leak_demo.c

use_after_free: use_after_free.c
	$(CC) $(CFLAGS) -o use_after_free use_after_free.c

double_free: double_free.c
	$(CC) $(CFLAGS) -o double_free double_free.c

clean:
	rm -f $(TARGETS)
