// swap_demo.c
#include <stdio.h>

static void swap_wrong(int a, int b) {     // hoán đổi BẢN SAO: không ảnh hưởng người gọi
    int t = a; a = b; b = t;
}

static void swap_int(int *a, int *b) {
    int t = *a; *a = *b; *b = t;
}

int main(void) {
    int x = 1, y = 2;
    swap_wrong(x, y);
    printf("sau swap_wrong: %d %d\n", x, y);    // 1 2
    swap_int(&x, &y);
    printf("sau swap_int:   %d %d\n", x, y);    // 2 1
    return 0;
}
