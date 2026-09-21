// ub_cases.c
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int which = argc > 1 ? atoi(argv[1]) : 0;
    volatile int zero = 0;                 // volatile: compiler không đoán trước được giá trị

    switch (which) {
    case 0: { volatile int x = INT_MAX; x = x + 1; printf("%d\n", x); break; }          /* tràn số có dấu */
    case 1: { int a[3] = {0}; volatile int i = 5; a[i] = 1; break; }                     /* ngoài mảng (stack) */
    case 2: { int *p = NULL; printf("%d\n", *p); break; }                                /* NULL dereference */
    case 3: { int *p = malloc(sizeof *p); free(p); *p = 1; break; }                      /* use-after-free */
    case 4: { int *p = malloc(sizeof *p); free(p); free(p); break; }                     /* double free */
    case 5: { int x; volatile int y = x; printf("%d\n", y); break; }                     /* chưa khởi tạo */
    case 6: { printf("%d\n", 1 / zero); break; }                                         /* chia cho 0 */
    case 7: { volatile int s = 32; printf("%d\n", 1 << s); break; }                      /* dịch bit quá rộng */
    case 8: { char *s = "abc"; s[0] = 'x'; break; }                                      /* ghi vào literal */
    default: puts("chon 0..8");
    }
    return 0;
}
