// temperature.c
#include <stdio.h>

int main(void) {
    double c;
    if (scanf("%lf", &c) != 1) return 1;
    printf("sai:  %.1f\n", c * (9 / 5) + 32);      // 9 / 5 = 1 (chia nguyên!) nên chỉ cộng 32
    printf("dung: %.1f\n", c * 9.0 / 5 + 32);
    return 0;
}
