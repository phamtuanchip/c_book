// parse_int_test.c
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_int(const char *s, int *out) {
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (end == s || strpbrk(s, "0123456789") == NULL) return -1;   // rỗng / không có số
    if (*end != '\0') return -1;                   // ký tự thừa (kể cả khoảng trắng cuối)
    if (errno == ERANGE || v < INT_MIN || v > INT_MAX) return -1;
    *out = (int)v;
    return 0;
}

int main(void) {
    struct { const char *in; int ok; int val; } T[] = {
        { "0", 1, 0 }, { "42", 1, 42 }, { "-42", 1, -42 }, { "+7", 1, 7 },
        { "2147483647", 1, INT_MAX }, { "-2147483648", 1, INT_MIN },
        { "2147483648", 0, 0 }, { "-2147483649", 0, 0 },
        { "", 0, 0 }, { "abc", 0, 0 }, { "12abc", 0, 0 }, { "12 ", 0, 0 },
        { " 12", 1, 12 },                            // strtol bỏ qua khoảng trắng ĐẦU — ghi rõ hành vi này
        { "99999999999999999999", 0, 0 },
    };
    int failed = 0;
    for (size_t i = 0; i < sizeof T / sizeof T[0]; i++) {
        int v = 0;
        int ok = parse_int(T[i].in, &v) == 0;
        if (ok != T[i].ok || (ok && v != T[i].val)) { printf("FAIL: \"%s\"\n", T[i].in); failed++; }
    }
    printf("%d that bai\n", failed);
    return failed != 0;
}
