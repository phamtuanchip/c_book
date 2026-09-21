// read_int.c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

// Đọc một số nguyên từ stdin. Trả về 0 nếu thành công, -1 nếu lỗi.
int read_int(const char *prompt, int *out) {
    char buf[64];
    printf("%s", prompt);
    if (fgets(buf, sizeof buf, stdin) == NULL) return -1;

    char *end;
    errno = 0;
    long v = strtol(buf, &end, 10);          // đổi chuỗi thành số, cơ số 10
    if (end == buf || strpbrk(buf, "0123456789") == NULL) return -1;   // không có chữ số nào
    if (*end != '\n' && *end != '\0') return -1;   // còn ký tự lạ phía sau
    if (errno == ERANGE || v < INT_MIN || v > INT_MAX) return -1;   // ngoài phạm vi int

    *out = (int)v;
    return 0;
}

int main(void) {
    int n;
    if (read_int("Nhap mot so nguyen: ", &n) != 0) {
        fprintf(stderr, "Du lieu khong hop le\n");
        return 1;
    }
    printf("Ban vua nhap: %d\n", n);
    return 0;
}
