// profile_demo.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Điểm nghẽn: đếm phần tử trùng bằng vòng lặp lồng nhau O(n^2)
static int count_duplicates(const int *a, int n) {
    int dups = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (a[i] == a[j]) dups++;
    return dups;
}

static long checksum(const int *a, int n) {
    long s = 0;
    for (int i = 0; i < n; i++) s += a[i];
    return s;
}

int main(void) {
    int n = 30000;
    int *a = malloc((size_t)n * sizeof *a);
    srand(42);
    for (int i = 0; i < n; i++) a[i] = rand() % 100000;

    printf("checksum = %ld\n", checksum(a, n));
    printf("duplicates = %d\n", count_duplicates(a, n));
    free(a);
    return 0;
}
