// use_after_free.c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int *p = malloc(sizeof *p);
    *p = 42;
    free(p);
    printf("%d\n", *p);      // LỖI: đọc vùng nhớ đã trả lại -> UB
    *p = 7;                  // còn tệ hơn: ghi -> có thể phá dữ liệu của phần khác
    return 0;
}
