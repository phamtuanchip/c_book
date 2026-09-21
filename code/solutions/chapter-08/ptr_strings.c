// ptr_strings.c
#include <stddef.h>
#include <stdio.h>

static size_t my_strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

static int my_strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

static char *my_strchr(const char *s, int c) {
    for (; *s; s++) if (*s == (char)c) return (char *)s;
    return c == 0 ? (char *)s : NULL;         // chuẩn C: tìm được cả ký tự '\0' kết thúc
}

static void my_strrev(char *s) {
    char *e = s;
    while (*e) e++;
    while (s < --e) {                          // e trỏ tới ký tự cuối, s tiến, e lùi
        char t = *s; *s++ = *e; *e = t;
    }
}

int main(void) {
    char s[] = "hello";
    printf("%zu %d %s\n", my_strlen(s), my_strcmp("abc", "abd"), my_strchr(s, 'l'));   // 5 -1 llo
    my_strrev(s);
    printf("%s\n", s);                                                                 // olleh
    return 0;
}
