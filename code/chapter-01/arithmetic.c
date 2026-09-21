// arithmetic.c
#include <stdio.h>

int main(void) {
    int a, b;

    printf("Nhap hai so nguyen (cach nhau bang dau cach): ");
    if (scanf("%d %d", &a, &b) != 2) {
        fprintf(stderr, "Loi: du lieu nhap khong hop le.\n");
        return 1;                    // mã khác 0 = có lỗi
    }

    printf("%d + %d = %d\n", a, b, a + b);
    printf("%d - %d = %d\n", a, b, a - b);
    printf("%d * %d = %d\n", a, b, a * b);

    if (b == 0) {
        printf("Khong the chia cho 0.\n");
    } else {
        printf("%d / %d = %d (phan nguyen)\n", a, b, a / b);
        printf("%d %% %d = %d (phan du)\n", a, b, a % b);
        printf("%d / %d = %.2f (so thuc)\n", a, b, (double)a / b);
    }
    return 0;
}
