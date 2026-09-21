// parse_ints.c
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// Đọc các số nguyên cách nhau bởi khoảng trắng; trả số lượng đọc được, hoặc -1 nếu gặp token sai
static int parse_ints(const char *s, int *out, int cap) {
    int n = 0;
    const char *p = s;
    for (;;) {
        while (isspace((unsigned char)*p)) p++;                // tự bỏ khoảng trắng: không dựa vào cách libc đặt `end`
        if (*p == '\0') return n;                              // hết chuỗi: thành công
        if (!isdigit((unsigned char)*p) && *p != '-' && *p != '+') return -1;   // token không phải số

        char *end;
        errno = 0;
        long v = strtol(p, &end, 10);
        if (end == p) return -1;                               // chỉ có dấu, không có chữ số
        if (errno == ERANGE || v < INT_MIN || v > INT_MAX || n >= cap) return -1;
        out[n++] = (int)v;
        p = end;
    }
}

int main(void) {
    int a[16];
    int n = parse_ints("10 20  30\n", a, 16);
    for (int i = 0; i < n; i++) printf("%d ", a[i]);           // 10 20 30
    printf("\n%d\n", parse_ints("1 2 x", a, 16));              // -1
    return 0;
}
