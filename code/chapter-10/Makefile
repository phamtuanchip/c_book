CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGETS = linked_list stack queue

all: $(TARGETS)

linked_list: linked_list.c
	$(CC) $(CFLAGS) -o linked_list linked_list.c

stack: stack.c
	$(CC) $(CFLAGS) -o stack stack.c

queue: queue.c
	$(CC) $(CFLAGS) -o queue queue.c

clean:
	rm -f $(TARGETS)
