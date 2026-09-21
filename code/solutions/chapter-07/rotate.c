// rotate.c
#include <stddef.h>
#include <stdio.h>

static void reverse_range(int *a, size_t lo, size_t hi) {       // đảo a[lo .. hi-1]
    while (lo + 1 < hi) {
        hi--;
        int t = a[lo]; a[lo] = a[hi]; a[hi] = t;
        lo++;
    }
}

// Xoay trái k vị trí: đảo [0,k), đảo [k,n), rồi đảo toàn bộ. O(n), O(1) bộ nhớ.
static void rotate_left(int *a, size_t n, size_t k) {
    if (n == 0) return;
    k %= n;
    reverse_range(a, 0, k);
    reverse_range(a, k, n);
    reverse_range(a, 0, n);
}

int main(void) {
    int a[] = {1, 2, 3, 4, 5, 6};
    rotate_left(a, 6, 2);
    for (int i = 0; i < 6; i++) printf("%d ", a[i]);            // 3 4 5 6 1 2
    printf("\n");
    return 0;
}
