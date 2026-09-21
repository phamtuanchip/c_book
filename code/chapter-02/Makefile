# Build các ví dụ của chương 2. Dùng: make | make asan | make clean
CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -g -O1
LDLIBS  := -lm
PROGS   := bases overflow float_trap char_is_number sizes endian memory_regions bin_convert ascii_table demo_malloc

.PHONY: all asan clean
all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

asan: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
asan: clean all

clean:
	rm -f $(PROGS) *.exe
