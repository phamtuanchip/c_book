#include "math.h"

int add_int(int a, int b) { return a + b; }
int sub_int(int a, int b) { return a - b; }
int mul_int(int a, int b) { return a * b; }
int div_int(int a, int b, int *out) {
    if (b == 0) return 1; // error
    *out = a / b;
    return 0;
}

/* internal helper: not exposed in header */
static int abs_int(int x) { return x < 0 ? -x : x; }
