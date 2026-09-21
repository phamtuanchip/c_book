// double_bytes.c
#include <stdio.h>

int main(void) {
    double d = 1.0;
    unsigned char *p = (unsigned char *)&d;
    for (size_t i = 0; i < sizeof d; i++) printf("%02x ", p[i]);
    printf("\n");
    return 0;
}
