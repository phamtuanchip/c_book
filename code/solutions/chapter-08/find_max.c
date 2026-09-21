// find_max.c
#include <stddef.h>
#include <stdio.h>

// Trả về con trỏ tới phần tử lớn nhất; NULL nếu n == 0. Không sao chép, người gọi có thể sửa qua con trỏ.
static int *find_max(int *a, size_t n) {
    if (n == 0) return NULL;
    int *best = a;
    for (int *p = a + 1; p < a + n; p++)
        if (*p > *best) best = p;
    return best;
}

int main(void) {
    int a[] = {3, 9, 2, 9, 5};
    int *m = find_max(a, 5);
    if (m) printf("max = %d tai chi so %td\n", *m, m - a);   // 9 tai chi so 1
    *m = 0;                                                    // sửa thẳng phần tử trong mảng
    return 0;
}
