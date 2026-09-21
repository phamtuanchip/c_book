// classify.c
#include <stdio.h>

int main(void) {
    int n;
    printf("Nhap mot so nguyen: ");
    if (scanf("%d", &n) != 1) {
        fprintf(stderr, "Khong phai so nguyen\n");
        return 1;
    }

    if (n > 0) {
        printf("%d la so duong\n", n);
    } else if (n < 0) {
        printf("%d la so am\n", n);
    } else {
        printf("So 0\n");
    }

    printf("%d la so %s\n", n, (n % 2 == 0) ? "chan" : "le");
    return 0;
}
