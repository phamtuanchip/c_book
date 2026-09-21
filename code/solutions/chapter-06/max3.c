// max3.c
#include <stdio.h>

static int max3(int a, int b, int c) {
    int m = a;
    if (b > m) m = b;
    if (c > m) m = c;
    return m;
}

static void min_max3(int a, int b, int c, int *min, int *max) {
    *min = *max = a;
    if (b < *min) *min = b;
    if (b > *max) *max = b;
    if (c < *min) *min = c;
    if (c > *max) *max = c;
}

int main(void) {
    int lo, hi;
    min_max3(4, -2, 9, &lo, &hi);
    printf("max3 = %d, min = %d, max = %d\n", max3(4, -2, 9), lo, hi);   // 9, -2, 9
    return 0;
}
