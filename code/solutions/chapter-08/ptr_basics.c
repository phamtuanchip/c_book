// ptr_basics.c
#include <stdio.h>

int main(void) {
    int a = 5;
    int *p = &a;
    printf("a  = %d\n", a);                  // 5
    printf("&a = %p\n", (void *)&a);         // địa chỉ của a
    printf("p  = %p\n", (void *)p);          // cùng giá trị với &a
    printf("*p = %d\n", *p);                 // 5
    printf("&p = %p\n", (void *)&p);         // địa chỉ của chính biến p (khác &a)
    return 0;
}
