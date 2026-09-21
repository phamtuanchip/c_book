// float_trap.c
#include <stdio.h>
#include <math.h>

int main(void) {
    double a = 0.1 + 0.2;
    printf("0.1 + 0.2 = %.20f\n", a);       // 0.30000000000000004441
    printf("a == 0.3 ? %s\n", a == 0.3 ? "dung" : "sai");   // sai!

    // Cách so sánh đúng: so sánh với một sai số nhỏ (epsilon)
    if (fabs(a - 0.3) < 1e-9) {
        printf("gan bang 0.3 (trong sai so cho phep)\n");
    }
    return 0;
}
