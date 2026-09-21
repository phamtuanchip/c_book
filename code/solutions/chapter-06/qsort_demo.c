// qsort_demo.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cmp_desc(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x < y) - (x > y);                                  // đảo dấu -> giảm dần
}

static int cmp_str(const void *a, const void *b) {
    const char *x = *(const char *const *)a;                   // phần tử là "con trỏ tới chuỗi"
    const char *y = *(const char *const *)b;
    return strcmp(x, y);
}

int main(void) {
    int a[] = {5, 2, 9, 1, 7};
    qsort(a, 5, sizeof a[0], cmp_desc);
    for (int i = 0; i < 5; i++) printf("%d ", a[i]);           // 9 7 5 2 1
    printf("\n");

    const char *names[] = {"Cuong", "An", "Binh"};
    qsort(names, 3, sizeof names[0], cmp_str);
    for (int i = 0; i < 3; i++) printf("%s ", names[i]);       // An Binh Cuong
    printf("\n");
    return 0;
}
