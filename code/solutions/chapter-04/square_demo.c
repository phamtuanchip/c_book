// square_demo.c
#include <stdio.h>

#define SQUARE(x) ((x) * (x))
static inline int square_int(int x) { return x * x; }

int main(void) {
    int i = 3;
    int j = 3;
    int k = square_int(j++);                       // j++ chỉ được tính một lần
    printf("square_int(j++) = %d, j = %d\n", k, j);    // 9, 4
    (void)i;
    // SQUARE(i++) mở rộng thành ((i++) * (i++)): sửa i hai lần giữa hai sequence point -> UB
    // int bad = SQUARE(i++);      // bỏ comment và biên dịch với -Wall để xem cảnh báo, thử -fsanitize=undefined
    return 0;
}
