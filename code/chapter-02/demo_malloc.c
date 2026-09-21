#include <stdio.h>
#include <stdlib.h>

int main(void) {
    size_t n = 0;
    printf("Nhap so phan tu can cap phat: ");
    if (scanf("%zu", &n) != 1) { fprintf(stderr, "Gia tri khong hop le.\n"); return 1; }
    int *arr = malloc(n * sizeof(int));
    if (!arr) { perror("malloc"); return 1; }
    for (size_t i = 0; i < n; ++i) arr[i] = (int)i;
    printf("Da cap phat %zu phan tu (tong = %d)\n", n, n>0 ? arr[n-1] + (int)n - 1 : 0);
    free(arr);
    return 0;
}
