// swap_macro.c
#include <stdio.h>

#define SWAP(type, a, b) do { type swap_tmp__ = (a); (a) = (b); (b) = swap_tmp__; } while (0)

int main(void) {
    int x = 1, y = 2;
    if (x < y)
        SWAP(int, x, y);          // hoạt động như MỘT câu lệnh: if/else không cần { }
    else
        puts("khong doi");
    printf("%d %d\n", x, y);      // 2 1
    return 0;
}
