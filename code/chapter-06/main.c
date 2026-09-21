#include <stdio.h>
#include "math.h"

/* Demonstrate library usage and static variable */
int call_count(void) {
    static int cnt = 0; // preserved across calls
    return ++cnt;
}

int main(void) {
    int a = 10, b = 3;
    int r;
    printf("add: %d\n", add_int(a,b));
    printf("sub: %d\n", sub_int(a,b));
    printf("mul: %d\n", mul_int(a,b));
    if (div_int(a,b,&r) == 0) printf("div: %d\n", r);

    printf("call_count: %d\n", call_count());
    printf("call_count: %d\n", call_count());

    return 0;
}
