// char_is_number.c
#include <stdio.h>

int main(void) {
    char c = 'A';
    printf("c = %c, ma = %d\n", c, c);      // c = A, ma = 65
    c = c + 1;
    printf("c + 1 = %c\n", c);              // B

    char digit = '7';
    int value = digit - '0';                // 7  (mẹo đổi ký tự số sang giá trị)
    printf("ky tu '%c' co gia tri so %d\n", digit, value);

    char lower = 'g';
    char upper = lower - ('a' - 'A');       // 'G'
    printf("%c -> %c\n", lower, upper);
    return 0;
}
