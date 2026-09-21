// dup_count.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return (double)t.tv_sec + (double)t.tv_nsec * 1e-9; }

/* Gốc: O(n^2). Đếm số cặp (i<j) có a[i]==a[j]. */
static long dup_naive(const int *a, int n) {
    long c = 0;
    for (int i = 0; i < n; i++) for (int j = i + 1; j < n; j++) if (a[i] == a[j]) c++;
    return c;
}

static int cmp_int(const void *x, const void *y) { int a = *(const int *)x, b = *(const int *)y; return (a > b) - (a < b); }

/* Sắp xếp + quét: O(n log n). Nhóm k phần tử bằng nhau cho k(k-1)/2 cặp. */
static long dup_sort(const int *src, int n) {
    int *a = malloc((size_t)n * sizeof *a);
    if (!a) return -1;
    memcpy(a, src, (size_t)n * sizeof *a);
    qsort(a, (size_t)n, sizeof *a, cmp_int);
    long pairs = 0;
    for (int i = 0; i < n; ) {
        int j = i;
        while (j < n && a[j] == a[i]) j++;
        long k = j - i;
        pairs += k * (k - 1) / 2;
        i = j;
    }
    free(a);
    return pairs;
}

/* Mảng đếm: O(n + K) khi giá trị nằm trong [0, K). */
#define K 100000
static long dup_count(const int *a, int n) {
    int *cnt = calloc(K, sizeof *cnt);
    if (!cnt) return -1;
    long pairs = 0;
    for (int i = 0; i < n; i++) pairs += cnt[a[i]]++;       // cnt hiện có = số phần tử bằng a[i] đã gặp
    free(cnt);
    return pairs;
}

int main(void) {
    for (int n = 10000; n <= 1000000; n *= 10) {
        int *a = malloc((size_t)n * sizeof *a);
        srand(42);
        for (int i = 0; i < n; i++) a[i] = rand() % K;

        double t0 = now();
        long r1 = n <= 100000 ? dup_naive(a, n) : -1;        // O(n^2) quá chậm với 10^6
        double t1 = now();
        long r2 = dup_sort(a, n);
        double t2 = now();
        long r3 = dup_count(a, n);
        double t3 = now();
        printf("n=%7d naive %8.3fs sort %.4fs count %.4fs  %s\n", n, t1 - t0, t2 - t1, t3 - t2,
               (r1 < 0 || r1 == r2) && r2 == r3 ? "ket qua khop" : "KHAC NHAU!");
        free(a);
    }
    return 0;
}
