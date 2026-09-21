// bases.c
#include <stdio.h>

int main(void) {
    int n = 255;
    printf("thap phan: %d\n", n);
    printf("thap luc : %x (viet hoa: %X, kem tien to: %#x)\n", n, n, n);
    printf("bat phan : %o\n", n);
    return 0;
}
