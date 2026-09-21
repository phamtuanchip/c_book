// read_range.c
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// Hỏi cho đến khi người dùng nhập số nguyên trong [lo, hi]. Trả 0 nếu thành công, -1 nếu hết đầu vào (EOF).
static int read_int_range(const char *prompt, int lo, int hi, int *out) {
    char buf[64];
    for (;;) {
        printf("%s [%d..%d]: ", prompt, lo, hi);
        fflush(stdout);
        if (!fgets(buf, sizeof buf, stdin)) return -1;           // EOF: dừng, tránh lặp vô hạn

        char *end;
        errno = 0;
        long v = strtol(buf, &end, 10);
        if (end == buf || (*end != '\n' && *end != '\0') || errno == ERANGE || v < lo || v > hi) {
            printf("Gia tri khong hop le, thu lai.\n");
            continue;
        }
        *out = (int)v;
        return 0;
    }
}

int main(void) {
    int age;
    if (read_int_range("Nhap tuoi", 1, 120, &age) != 0) return 1;
    printf("Tuoi: %d\n", age);
    return 0;
}
