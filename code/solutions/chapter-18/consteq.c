// consteq.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <time.h>

static int consteq(const void *a, const void *b, size_t n) {
    const unsigned char *x = a, *y = b;
    unsigned char diff = 0;
    for (size_t i = 0; i < n; i++) diff |= x[i] ^ y[i];
    return diff == 0;
}

static double bench(int (*cmp)(const void *, const void *, size_t), const char *x, const char *y, size_t n) {
    struct timespec a, b;
    volatile int sink = 0;
    clock_gettime(CLOCK_MONOTONIC, &a);
    for (int i = 0; i < 20000000; i++) sink += cmp(x, y, n);
    clock_gettime(CLOCK_MONOTONIC, &b);
    (void)sink;
    return (double)(b.tv_sec - a.tv_sec) + (double)(b.tv_nsec - a.tv_nsec) * 1e-9;
}

static int memcmp_wrap(const void *a, const void *b, size_t n) { return memcmp(a, b, n) == 0; }

int main(void) {
    char a[64], b_first[64], b_last[64];
    memset(a, 'x', sizeof a); memcpy(b_first, a, sizeof a); memcpy(b_last, a, sizeof a);
    b_first[0] = 'y';                      // khác ở byte ĐẦU
    b_last[63] = 'y';                      // khác ở byte CUỐI
    printf("memcmp  đầu %.3f  cuối %.3f\n", bench(memcmp_wrap, a, b_first, 64), bench(memcmp_wrap, a, b_last, 64));
    printf("consteq đầu %.3f  cuối %.3f\n", bench(consteq, a, b_first, 64), bench(consteq, a, b_last, 64));
    return 0;
}
