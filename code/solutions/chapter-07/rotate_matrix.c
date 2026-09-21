// rotate_matrix.c
#include <stdio.h>

#define N 3

static void rotate_cw(int m[N][N]) {
    for (int i = 0; i < N; i++)                  // chuyển vị
        for (int j = i + 1; j < N; j++) {
            int t = m[i][j]; m[i][j] = m[j][i]; m[j][i] = t;
        }
    for (int i = 0; i < N; i++)                  // đảo từng hàng
        for (int j = 0; j < N / 2; j++) {
            int t = m[i][j]; m[i][j] = m[i][N - 1 - j]; m[i][N - 1 - j] = t;
        }
}

int main(void) {
    int m[N][N] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    rotate_cw(m);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) printf("%d ", m[i][j]);
        printf("\n");                            // 7 4 1 / 8 5 2 / 9 6 3
    }
    return 0;
}
