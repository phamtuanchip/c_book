// arith_safe.c
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

// Đọc một số nguyên từ một dòng; trả 0 nếu hợp lệ
static int read_int(const char *prompt, int *out) {
    char buf[64];
    printf("%s", prompt);
    if (fgets(buf, sizeof buf, stdin) == NULL) return -1;

    char *end;
    errno = 0;
    long v = strtol(buf, &end, 10);
    if (end == buf || strpbrk(buf, "0123456789") == NULL || (*end != '\n' && *end != '\0')) return -1;   // không phải số / có ký tự thừa
    if (errno == ERANGE || v < INT_MIN || v > INT_MAX) return -1;  // ngoài phạm vi int
    *out = (int)v;
    return 0;
}

int main(void) {
    int a, b;
    if (read_int("a = ", &a) != 0 || read_int("b = ", &b) != 0) {
        fprintf(stderr, "Du lieu khong hop le\n");
        return 1;
    }

    // Dùng long long để phép tính không tràn với a, b lớn
    printf("%d + %d = %lld\n", a, b, (long long)a + b);
    printf("%d - %d = %lld\n", a, b, (long long)a - b);
    printf("%d * %d = %lld\n", a, b, (long long)a * b);

    if (b == 0) {
        printf("Khong the chia cho 0\n");
    } else {
        printf("%d / %d = %d (phan nguyen)\n", a, b, a / b);
        printf("%d %% %d = %d (phan du)\n", a, b, a % b);
        printf("%d / %d = %.3f (so thuc)\n", a, b, (double)a / b);
    }
    return 0;
}
