// sort_bench.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "timer.h"

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

// Sắp xếp chèn: O(n^2), tốt cho mảng nhỏ / gần sắp xếp
static void insertion_sort(int *a, size_t n) {
    for (size_t i = 1; i < n; i++) {
        int k = a[i]; size_t j = i;
        while (j > 0 && a[j - 1] > k) { a[j] = a[j - 1]; j--; }
        a[j] = k;
    }
}

// Sắp xếp nhanh (quicksort), chọn pivot giữa, đệ quy trên phần nhỏ hơn để giới hạn độ sâu
static void quicksort(int *a, long lo, long hi) {
    while (lo < hi) {
        int pivot = a[lo + (hi - lo) / 2];
        long i = lo, j = hi;
        while (i <= j) {
            while (a[i] < pivot) i++;
            while (a[j] > pivot) j--;
            if (i <= j) { int t = a[i]; a[i] = a[j]; a[j] = t; i++; j--; }
        }
        if (j - lo < hi - i) { quicksort(a, lo, j); lo = i; }     // đệ quy phần nhỏ, lặp phần lớn
        else                 { quicksort(a, i, hi); hi = j; }
    }
}

// Sắp xếp trộn (mergesort): O(n log n) ổn định, cần bộ nhớ phụ
static void merge_rec(int *a, int *tmp, size_t n) {
    if (n < 2) return;
    size_t mid = n / 2;
    merge_rec(a, tmp, mid);
    merge_rec(a + mid, tmp, n - mid);
    size_t i = 0, j = mid, k = 0;
    while (i < mid && j < n) tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    while (i < mid) tmp[k++] = a[i++];
    while (j < n)   tmp[k++] = a[j++];
    memcpy(a, tmp, n * sizeof *a);
}
static void mergesort_ints(int *a, size_t n) {
    int *tmp = malloc(n * sizeof *tmp);
    if (!tmp) return;
    merge_rec(a, tmp, n);
    free(tmp);
}

static int is_sorted(const int *a, size_t n) {
    for (size_t i = 1; i < n; i++) if (a[i - 1] > a[i]) return 0;
    return 1;
}

int main(void) {
    size_t n = 2000000;
    int *orig = malloc(n * sizeof *orig), *work = malloc(n * sizeof *work);
    if (!orig || !work) return 1;
    srand(12345);                                        // seed cố định -> dữ liệu lặp lại được
    for (size_t i = 0; i < n; i++) orig[i] = rand();

    struct { const char *name; } algos[] = { {"qsort (libc)"}, {"quicksort"}, {"mergesort"} };
    for (int k = 0; k < 3; k++) {
        memcpy(work, orig, n * sizeof *work);            // dữ liệu giống hệt nhau cho mọi thuật toán
        double t0 = now_seconds();
        if (k == 0) qsort(work, n, sizeof *work, cmp_int);
        else if (k == 1) quicksort(work, 0, (long)n - 1);
        else mergesort_ints(work, n);
        double dt = now_seconds() - t0;
        printf("%-14s %8.1f ms  %s\n", algos[k].name, dt * 1e3, is_sorted(work, n) ? "OK" : "SAI!");
    }
    (void)insertion_sort;                                // dùng cho thử nghiệm mảng nhỏ ở bài tập
    free(orig); free(work);
    return 0;
}
