// dec2bin.c
#include <stdio.h>

int main(void) {
    unsigned n;
    if (scanf("%u", &n) != 1) return 1;
    if (n == 0) { puts("0"); return 0; }

    char bits[33];
    int len = 0;
    while (n > 0) {
        bits[len++] = (char)('0' + (n % 2));      // các bit ra theo thứ tự ngược
        n /= 2;
    }
    for (int i = len - 1; i >= 0; i--) putchar(bits[i]);
    putchar('\n');
    return 0;
}
