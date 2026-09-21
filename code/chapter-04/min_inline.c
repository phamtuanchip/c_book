#include <stdio.h>
#include "min.h"

int main(void) {
    int a = 10, b = 7;
    printf("min_int(%d, %d) = %d\n", a, b, min_int(a,b));
    long la = 100L, lb = 200L;
    printf("min_long(%ld, %ld) = %ld\n", la, lb, min_long(la, lb));
    return 0;
}
