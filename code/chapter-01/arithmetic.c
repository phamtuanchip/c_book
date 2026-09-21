#include <stdio.h>

int main(void) {
    int a, b;
    printf("Nhap hai so nguyen: ");
    if (scanf("%d %d", &a, &b) != 2) {
        printf("Nhap khong hop le.\n");
        return 1;
    }
    printf("%d + %d = %d\n", a, b, a + b);
    printf("%d - %d = %d\n", a, b, a - b);
    printf("%d * %d = %d\n", a, b, a * b);
    if (b != 0) {
        printf("%d / %d = %d\n", a, b, a / b);
    } else {
        printf("Khong the chia cho 0.\n");
    }
    return 0;
}
