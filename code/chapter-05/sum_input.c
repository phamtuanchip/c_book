// sum_input.c
#include <stdio.h>

int main(void) {
    long sum = 0;
    int x;
    int count = 0;

    while (scanf("%d", &x) == 1) {      // dừng khi không đọc được số nữa (EOF hoặc chữ)
        sum += x;
        count++;
    }

    if (count > 0) {
        printf("count = %d, sum = %ld, average = %.2f\n", count, sum, (double)sum / count);
    } else {
        printf("khong co so nao\n");
    }
    return 0;
}
