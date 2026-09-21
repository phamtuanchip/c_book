// power.c
#include <stdio.h>

// O(e): nhân e lần
static long power_slow(long base, unsigned exp) {
    if (exp == 0) return 1;
    return base * power_slow(base, exp - 1);
}

// O(log e): power(b, e) = power(b, e/2)^2 (nhân thêm b nếu e lẻ)
static long power_fast(long base, unsigned exp) {
    if (exp == 0) return 1;
    long half = power_fast(base, exp / 2);
    return (exp % 2 == 0) ? half * half : half * half * base;
}

int main(void) {
    printf("%ld %ld\n", power_slow(3, 13), power_fast(3, 13));    // 1594323 1594323
    return 0;
}
