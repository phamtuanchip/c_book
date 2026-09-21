#include <stdio.h>

int main(void) {
    printf("ASCII printable characters (decimal, hex, char):\n");
    for (int c = 32; c <= 126; ++c) {
        printf("%3d 0x%02X %c\n", c, c, (char)c);
    }
    return 0;
}
