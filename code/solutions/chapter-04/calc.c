// calc.c
#include "calc.h"

int calc_add(int a, int b) { return a + b; }
int calc_sub(int a, int b) { return a - b; }
int calc_mul(int a, int b) { return a * b; }

int calc_div_safe(int a, int b, int *out) {
    if (b == 0) return -1;
    *out = a / b;
    return 0;
}
