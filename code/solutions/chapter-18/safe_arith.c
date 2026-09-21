// safe_arith.c
#include <limits.h>
#include <stdio.h>

static int safe_add(int a, int b, int *out) {
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b)) return -1;
    *out = a + b;
    return 0;
}

static int safe_mul(int a, int b, int *out) {
    if (a == 0 || b == 0) { *out = 0; return 0; }
    if ((a == -1 && b == INT_MIN) || (b == -1 && a == INT_MIN)) return -1;
    if (a > 0 ? (b > 0 ? a > INT_MAX / b : b < INT_MIN / a)
              : (b > 0 ? a < INT_MIN / b : b < INT_MAX / a)) return -1;
    *out = a * b;
    return 0;
}

int main(void) {
    int r;
    printf("%d %d\n", safe_add(INT_MAX, 1, &r), safe_add(INT_MAX - 1, 1, &r));       // -1 0
    printf("%d %d\n", safe_add(INT_MIN, -1, &r), safe_mul(INT_MIN, -1, &r));         // -1 -1
    printf("%d %d\n", safe_mul(46341, 46341, &r), safe_mul(46340, 46340, &r));       // -1 0
    return 0;
}
