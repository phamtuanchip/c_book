// minmax_macro.c
#include <stdio.h>

#define MIN(a, b)        ((a) < (b) ? (a) : (b))
#define MAX(a, b)        ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) MIN(MAX((x), (lo)), (hi))

static inline int clamp_int(int x, int lo, int hi) { return x < lo ? lo : (x > hi ? hi : x); }

int main(void) {
    int i = 5;
    int a = clamp_int(i++, 0, 10);              // i++ đúng MỘT lần
    printf("clamp_int: %d, i = %d\n", a, i);    // 5, 6
    // CLAMP(i++, 0, 10) sẽ đánh giá i++ tới bốn lần (MIN/MAX mở rộng mỗi tham số nhiều lần) -> UB
    printf("%d\n", CLAMP(20, 0, 10));           // 10 (an toàn vì đối số không có tác dụng phụ)
    return 0;
}
