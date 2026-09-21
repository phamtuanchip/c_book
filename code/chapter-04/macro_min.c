#include <stdio.h>
#include "min.h"

#define MIN_MACRO(a,b) ((a) < (b) ? (a) : (b))

int main(void) {
    int x = 3;
    int y = 4;
    printf("MIN_MACRO(x++, y) = %d\n", MIN_MACRO(x++, y));
    // x may be incremented more than once due to macro expansion
    printf("After macro, x = %d\n", x);

    // Using inline function is safer
    x = 3; y = 4;
    printf("min_int(x++, y) = %d\n", min_int(x++, y));
    printf("After inline, x = %d\n", x);
    return 0;
}
