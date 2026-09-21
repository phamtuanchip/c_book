// limits_demo.c
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    printf("INT_MAX   = %d\n", INT_MAX);
    printf("INT_MIN   = %d\n", INT_MIN);
    printf("LONG_MAX  = %ld\n", LONG_MAX);
    printf("SIZE_MAX  = %zu\n", (size_t)SIZE_MAX);
    return 0;
}
