// stats_stdin.c
#include <stdio.h>

int main(void) {
    int x, count = 0, min = 0, max = 0;
    long sum = 0;
    while (scanf("%d", &x) == 1) {
        if (count == 0 || x < min) min = x;
        if (count == 0 || x > max) max = x;
        sum += x;
        count++;
    }
    if (count == 0) { printf("khong co so nao\n"); return 0; }
    printf("min = %d, max = %d, trung binh = %.2f\n", min, max, (double)sum / count);
    return 0;
}
