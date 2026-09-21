// compat_test.c
#include <stdio.h>

int main(void) {
    // "long long" và "//" comment: C99 trở lên
    long long big = 9000000000LL;
    printf("big = %lld\n", big);

    // Macro cho biết chuẩn mà compiler đang dùng
    #ifdef __STDC_VERSION__
        printf("__STDC_VERSION__ = %ld\n", __STDC_VERSION__);
    #else
        printf("C89/C90 (khong co __STDC_VERSION__)\n");
    #endif
    return 0;
}
