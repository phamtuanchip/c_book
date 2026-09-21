# ---- Cấu hình ----
CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -g -O0 -Iinclude
LDFLAGS :=
LDLIBS  := -lm

SRC_DIR   := src
BUILD_DIR := build
TARGET    := $(BUILD_DIR)/app

SRCS := $(wildcard $(SRC_DIR)/*.c)                     # mọi file .c trong src/
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)                                  # file phụ thuộc tự sinh

# ---- Mục tiêu ----
.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Quy tắc mẫu: mỗi src/X.c -> build/X.o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./tests/run_tests.sh

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
