#include <stdio.h>

#define LOG(level, fmt, ...) do { \
    fprintf(stderr, "%s:%d: %s: " fmt "\n", __FILE__, __LINE__, level, ##__VA_ARGS__); \
} while (0)

int main(void) {
    LOG("INFO", "Program started");
    int x = 5;
    LOG("DEBUG", "x = %d", x);
    return 0;
}
