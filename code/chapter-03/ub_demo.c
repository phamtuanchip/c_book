// ub_demo.c
#include <stdio.h>
#include <limits.h>

int is_next_bigger(int x) {
    return x + 1 > x;      // với số có dấu, tràn số là UB
}

int main(void) {
    printf("%d\n", is_next_bigger(INT_MAX));
    return 0;
}
