# Build các ví dụ của chương 4. Dùng: make | make asan | make clean
CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -g -O1
LDLIBS  := -lm
SRCS := $(wildcard src/*.c)

.PHONY: all asan clean run
all: build/app

build/app: $(SRCS) | build
	$(CC) $(CFLAGS) -Iinclude $(SRCS) -o $@ $(LDLIBS)

build:
	mkdir -p build

run: build/app
	./build/app

asan: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
asan: clean all

clean:
	rm -rf build
