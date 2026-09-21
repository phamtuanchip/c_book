#include <stdio.h>

int main(void) {
    int N;
    printf("Nhap N (1..N): ");
    if (scanf("%d", &N) != 1 || N < 1) { fprintf(stderr, "Nhap khong hop le.\n"); return 1; }
    for (int i = 1; i <= N; ++i) {
        for (int j = 1; j <= N; ++j) {
            printf("%4d", i*j);
        }
        putchar('\n');
    }
    return 0;
}
