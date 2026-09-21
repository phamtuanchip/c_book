// classify_char.c
#include <ctype.h>
#include <stdio.h>

// Đổi chữ hoa thành chữ thường không dùng tolower: 'A' (65) và 'a' (97) chênh 32
static char to_lower_ascii(char c) {
    if (c >= 'A' && c <= 'Z') return (char)(c + ('a' - 'A'));
    return c;
}

int main(void) {
    int c = getchar();
    if (c == EOF) return 1;
    unsigned char ch = (unsigned char)c;

    if (isupper(ch))      printf("'%c' la chu hoa\n", ch);
    else if (islower(ch)) printf("'%c' la chu thuong\n", ch);
    else if (isdigit(ch)) printf("'%c' la chu so\n", ch);
    else                  printf("ky tu khac (ma %d)\n", ch);

    printf("chu thuong: %c\n", to_lower_ascii((char)ch));
    return 0;
}
