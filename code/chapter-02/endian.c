// endian.c
#include <stdio.h>

int main(void) {
    unsigned int x = 0x12345678;
    unsigned char *p = (unsigned char *)&x;   // nhìn x như dãy byte

    for (size_t i = 0; i < sizeof(x); i++) {
        printf("byte %zu: 0x%02x\n", i, p[i]);
    }
    printf("May nay la %s-endian\n", p[0] == 0x78 ? "little" : "big");
    return 0;
}
