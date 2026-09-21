#include <stdio.h>

void demo_basic(void) {
    int x = 42;
    int *p = &x;
    printf("x = %d, *p = %d, address p = %p\n", x, *p, (void*)p);
    *p = 100;
    printf("after *p=100, x = %d\n", x);
}

void demo_array(void) {
    int a[5] = {1,2,3,4,5};
    int *p = a; // same as &a[0]
    for (int i = 0; i < 5; ++i) {
        printf("a[%d]=%d, *(p+%d)=%d\n", i, a[i], i, *(p+i));
    }
}

int main(void) {
    demo_basic();
    demo_array();
    return 0;
}
