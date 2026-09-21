#include <stdio.h>
#include <stdlib.h>

int safe_scan_int(int *out) {
    char buf[64];
    if (fgets(buf, sizeof(buf), stdin) == NULL) return 0;
    char *endptr;
    long v = strtol(buf, &endptr, 10);
    if (endptr == buf || (*endptr != '\n' && *endptr != '\0')) return 0;
    *out = (int)v;
    return 1;
}

int main(void) {
    int a, b;
    printf("Nhap hai so nguyen (dong 1): ");
    if (!safe_scan_int(&a)) { printf("Gia tri khong hop le.\n"); return 1; }
    if (!safe_scan_int(&b)) { printf("Gia tri khong hop le.\n"); return 1; }
    printf("%d + %d = %d\n", a, b, a + b);
    return 0;
}
