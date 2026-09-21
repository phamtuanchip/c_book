#include <stdio.h>
#include <stdlib.h>

void print_binary(unsigned int v) {
    if (v == 0) { printf("0\n"); return; }
    unsigned int bits = sizeof(v) * 8;
    int started = 0;
    for (int i = bits - 1; i >= 0; --i) {
        unsigned int mask = 1u << i;
        int bit = (v & mask) ? 1 : 0;
        if (bit) started = 1;
        if (started) putchar(bit ? '1' : '0');
    }
    putchar('\n');
}

int main(void) {
    unsigned int x;
    printf("Nhap mot so nguyen duong (unsigned): ");
    if (scanf("%u", &x) != 1) {
        fprintf(stderr, "Gia tri khong hop le.\n");
        return 1;
    }
    printf("Binary: ");
    print_binary(x);
    return 0;
}
