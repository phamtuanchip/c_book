#include <stdio.h>
#include <stdlib.h>

int cmp_int(const void *a, const void *b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

int main(void) {
    int arr[] = {5,2,9,1,7,3};
    size_t n = sizeof(arr)/sizeof(arr[0]);
    qsort(arr, n, sizeof(int), cmp_int);
    for (size_t i = 0; i < n; ++i) printf("%d ", arr[i]);
    printf("\n");
    return 0;
}
