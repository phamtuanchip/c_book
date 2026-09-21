// merge_sorted.c
#include <stddef.h>
#include <stdio.h>

// out phải có chỗ cho na + nb phần tử
static void merge_sorted(const int *a, size_t na, const int *b, size_t nb, int *out) {
    size_t i = 0, j = 0, k = 0;
    while (i < na && j < nb) out[k++] = (a[i] <= b[j]) ? a[i++] : b[j++];
    while (i < na) out[k++] = a[i++];
    while (j < nb) out[k++] = b[j++];
}

int main(void) {
    int a[] = {1, 4, 9}, b[] = {2, 3, 10, 11}, out[7];
    merge_sorted(a, 3, b, 4, out);
    for (int i = 0; i < 7; i++) printf("%d ", out[i]);           // 1 2 3 4 9 10 11
    printf("\n");
    return 0;
}
