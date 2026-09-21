// trim.c
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// Xóa khoảng trắng đầu và cuối, sửa tại chỗ; trả về chính s
static char *trim(char *s) {
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    size_t len = strlen(start);
    while (len > 0 && isspace((unsigned char)start[len - 1])) len--;
    memmove(s, start, len);                 // memmove vì hai vùng có thể chồng lấn
    s[len] = '\0';
    return s;
}

int main(void) {
    char s[] = "   xin chao  \n";
    printf("[%s]\n", trim(s));              // [xin chao]
    return 0;
}
