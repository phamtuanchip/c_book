# Build các ví dụ của chương 9. Dùng: make | make asan | make clean
CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -g -O1
LDLIBS  := -lm
PROGS   := leak_demo use_after_free vec tree

.PHONY: all asan clean
all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

asan: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
asan: clean all

clean:
	rm -f $(PROGS) *.exe
