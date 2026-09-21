// sort3.c
#include <stdio.h>

static void swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }

static void sort3(int *a, int *b, int *c) {
    if (*a > *b) swap(a, b);
    if (*b > *c) swap(b, c);
    if (*a > *b) swap(a, b);           // sau hai lượt, *c đã là lớn nhất; sắp lại a, b
}

int main(void) {
    int x = 3, y = 1, z = 2;
    sort3(&x, &y, &z);
    printf("%d %d %d\n", x, y, z);      // 1 2 3
    return 0;
}
