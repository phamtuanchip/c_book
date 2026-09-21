// next_id.c
#include <stdio.h>

static int next_id(void) {
    static int id = 0;            // giữ giá trị giữa các lần gọi, chỉ khởi tạo một lần
    return ++id;
}

int g_id = 0;                     // phiên bản toàn cục
static int next_id_global(void) { return ++g_id; }

int main(void) {
    int a = next_id();
    int b = next_id();
    int c = next_id();
    printf("%d %d %d\n", a, b, c);                 // 1 2 3
    a = next_id_global();
    b = next_id_global();
    printf("%d %d\n", a, b);                       // 1 2
    return 0;
}
