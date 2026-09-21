// circle.c
#include <stdio.h>

int main(void) {
    const double PI = 3.14159265358979323846;
    double r;
    printf("Ban kinh: ");
    if (scanf("%lf", &r) != 1 || r < 0) {          // %lf khi ĐỌC double
        fprintf(stderr, "Ban kinh khong hop le\n");
        return 1;
    }
    printf("Chu vi    = %.2f\n", 2 * PI * r);
    printf("Dien tich = %.2f\n", PI * r * r);
    return 0;
}
