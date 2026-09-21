// overflow.c
#include <stdio.h>
#include <limits.h>

int main(void) {
    unsigned char u = 255;
    u = u + 1;                       // quay vòng, hợp lệ
    printf("unsigned char 255 + 1 = %d\n", u);   // 0

    int big = INT_MAX;               // 2147483647
    printf("INT_MAX = %d\n", big);
    // big + 1 là UB: KHÔNG làm điều này trong mã thật.
    // Cách kiểm tra an toàn: kiểm tra TRƯỚC khi cộng
    if (big > INT_MAX - 1) {
        printf("cong them 1 se bi tran\n");
    }
    return 0;
}
