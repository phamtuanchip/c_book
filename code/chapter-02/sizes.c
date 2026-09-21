// sizes.c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    printf("char        : %zu\n", sizeof(char));         // luôn là 1
    printf("short       : %zu\n", sizeof(short));
    printf("int         : %zu\n", sizeof(int));
    printf("long        : %zu\n", sizeof(long));
    printf("long long   : %zu\n", sizeof(long long));
    printf("float       : %zu\n", sizeof(float));
    printf("double      : %zu\n", sizeof(double));
    printf("void*       : %zu\n", sizeof(void *));       // kích thước một con trỏ
    printf("int32_t     : %zu\n", sizeof(int32_t));      // luôn là 4
    printf("int64_t     : %zu\n", sizeof(int64_t));      // luôn là 8

    int arr[10];
    printf("arr         : %zu byte, %zu phan tu\n", sizeof(arr), sizeof(arr) / sizeof(arr[0]));
    return 0;
}
