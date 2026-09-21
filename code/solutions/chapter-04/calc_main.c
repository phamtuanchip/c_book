// calc_main.c
#include <stdio.h>
#include "calc.h"

int main(void) {
    int q;
    printf("7 + 2 = %d\n", calc_add(7, 2));
    printf("7 * 2 = %d\n", calc_mul(7, 2));
    if (calc_div_safe(7, 0, &q) != 0) printf("khong chia duoc cho 0\n");
    if (calc_div_safe(7, 2, &q) == 0) printf("7 / 2 = %d\n", q);
    return 0;
}
