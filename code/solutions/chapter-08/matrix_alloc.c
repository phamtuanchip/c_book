// matrix_alloc.c
#include <stdio.h>
#include <stdlib.h>

// Cách 1: mảng con trỏ hàng — dễ dùng m[i][j], nhưng nhiều lần cấp phát
static int **alloc_rows(int rows, int cols) {
    int **m = malloc((size_t)rows * sizeof *m);
    if (!m) return NULL;
    for (int i = 0; i < rows; i++) {
        m[i] = calloc((size_t)cols, sizeof **m);
        if (!m[i]) {
            while (i--) free(m[i]);                    // dọn những hàng đã cấp
            free(m);
            return NULL;
        }
    }
    return m;
}

static void free_rows(int **m, int rows) {
    if (!m) return;
    for (int i = 0; i < rows; i++) free(m[i]);
    free(m);
}

// Cách 2: một khối liên tiếp rows*cols — nhanh hơn (cache), giải phóng một lần; truy cập a[i * cols + j]
static int *alloc_flat(int rows, int cols) {
    return calloc((size_t)rows * (size_t)cols, sizeof(int));
}

int main(void) {
    int **m = alloc_rows(3, 4);
    if (!m) return 1;
    m[1][2] = 7;
    printf("%d\n", m[1][2]);
    free_rows(m, 3);

    int *f = alloc_flat(3, 4);
    if (!f) return 1;
    f[1 * 4 + 2] = 7;
    printf("%d\n", f[1 * 4 + 2]);
    free(f);
    return 0;
}
