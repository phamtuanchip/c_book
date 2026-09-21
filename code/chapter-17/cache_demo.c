// cache_demo.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

#define N 4096

int main(void) {
    int *m = malloc((size_t)N * N * sizeof *m);        // 64 MB (4096*4096*4)
    if (!m) return 1;
    for (size_t i = 0; i < (size_t)N * N; i++) m[i] = 1;

    volatile long sink;
    double t0 = now_seconds();
    long sum = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            sum += m[(size_t)i * N + j];               // theo hàng: liên tiếp
    sink = sum;
    double t_row = now_seconds() - t0;

    t0 = now_seconds();
    sum = 0;
    for (int j = 0; j < N; j++)
        for (int i = 0; i < N; i++)
            sum += m[(size_t)i * N + j];               // theo cột: nhảy cóc N*4 byte
    sink = sum;
    double t_col = now_seconds() - t0;

    (void)sink;
    printf("theo hang: %.1f ms\ntheo cot : %.1f ms (cham hon %.1fx)\n",
           t_row * 1e3, t_col * 1e3, t_col / t_row);
    free(m);
    return 0;
}
