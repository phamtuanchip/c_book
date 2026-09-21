// bitops.c
#include <stdint.h>
#include <stdio.h>

static uint32_t set_bit(uint32_t x, unsigned n)    { return x | (UINT32_C(1) << n); }
static uint32_t clear_bit(uint32_t x, unsigned n)  { return x & ~(UINT32_C(1) << n); }
static uint32_t toggle_bit(uint32_t x, unsigned n) { return x ^ (UINT32_C(1) << n); }
static int      test_bit(uint32_t x, unsigned n)   { return (int)((x >> n) & 1u); }

int main(void) {
    uint32_t x = 0;
    x = set_bit(x, 3);               // 0b1000
    x = set_bit(x, 0);               // 0b1001
    x = toggle_bit(x, 1);            // 0b1011
    x = clear_bit(x, 3);             // 0b0011
    printf("x = %u, bit0 = %d, bit3 = %d\n", x, test_bit(x, 0), test_bit(x, 3));   // 3, 1, 0
    return 0;
}
