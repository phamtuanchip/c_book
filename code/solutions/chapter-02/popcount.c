// popcount.c
#include <stdint.h>
#include <stdio.h>

// Cách 1: kiểm tra từng bit
static int popcount_loop(uint32_t x) {
    int c = 0;
    while (x) {
        c += (int)(x & 1u);
        x >>= 1;
    }
    return c;
}

// Cách 2 (Kernighan): mỗi lần x & (x - 1) xóa đúng một bit 1 thấp nhất
static int popcount_kernighan(uint32_t x) {
    int c = 0;
    while (x) {
        x &= x - 1;
        c++;
    }
    return c;
}

int main(void) {
    uint32_t tests[] = {0, 1, 13, 255, 0xFFFFFFFFu, 0x80000000u};
    for (size_t i = 0; i < sizeof tests / sizeof tests[0]; i++) {
        printf("%10u: %d %d\n", tests[i], popcount_loop(tests[i]), popcount_kernighan(tests[i]));
    }
    return 0;
}
