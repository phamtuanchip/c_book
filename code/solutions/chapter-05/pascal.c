// pascal.c
#include <stdio.h>

int main(void) {
    int rows = 8;
    for (int n = 0; n < rows; n++) {
        for (int s = 0; s < (rows - n - 1) * 2; s++) putchar(' ');   // căn giữa
        long c = 1;                                                   // C(n, 0)
        for (int k = 0; k <= n; k++) {
            printf("%4ld", c);
            c = c * (n - k) / (k + 1);                                // C(n, k+1) = C(n, k) * (n - k) / (k + 1)
        }
        printf("\n");
    }
    return 0;
}
