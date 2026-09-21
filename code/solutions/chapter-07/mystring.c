// mystring.c
#include <stddef.h>
#include <stdio.h>

static size_t my_strlen(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

// Sao chép tối đa cap-1 ký tự và luôn kết thúc bằng '\0'. Trả 0 nếu vừa, -1 nếu bị cắt.
static int my_strcpy(char *dst, size_t cap, const char *src) {
    if (cap == 0) return -1;
    size_t i = 0;
    for (; i + 1 < cap && src[i]; i++) dst[i] = src[i];
    dst[i] = '\0';
    return src[i] == '\0' ? 0 : -1;
}

static int my_strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

// Nối src vào dst (dst có kích thước cap). Trả 0 nếu vừa, -1 nếu bị cắt/không hợp lệ.
static int my_strcat(char *dst, size_t cap, const char *src) {
    size_t len = my_strlen(dst);
    if (len >= cap) return -1;
    return my_strcpy(dst + len, cap - len, src);
}

int main(void) {
    char buf[8] = "ab";
    printf("%zu\n", my_strlen(buf));                    // 2
    printf("%d\n", my_strcat(buf, sizeof buf, "cdef")); // 0  (buf = "abcdef")
    printf("%d\n", my_strcat(buf, sizeof buf, "xyz"));  // -1 (bị cắt: "abcdefx")
    printf("%s %d\n", buf, my_strcmp("abc", "abd"));    // abcdefx -1
    return 0;
}
