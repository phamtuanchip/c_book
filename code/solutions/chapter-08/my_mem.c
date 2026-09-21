// my_mem.c
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static void *my_memcpy(void *dst, const void *src, size_t n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;                 // KHÔNG đúng nếu hai vùng chồng lấn
    return dst;
}

static void *my_memmove(void *dst, const void *src, size_t n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    if (d == s || n == 0) return dst;
    if (d < s) {                             // đích ở trước nguồn: chép xuôi an toàn
        while (n--) *d++ = *s++;
    } else {                                 // đích ở sau nguồn (có thể chồng lấn): chép ngược
        d += n; s += n;
        while (n--) *--d = *--s;
    }
    return dst;
}

int main(void) {
    char buf[] = "abcdef";
    my_memmove(buf + 2, buf, 4);             // chồng lấn: "ababcd"
    printf("%s\n", buf);
    char b2[8];
    my_memcpy(b2, "xyz", 4);
    printf("%s\n", b2);
    return 0;
}
