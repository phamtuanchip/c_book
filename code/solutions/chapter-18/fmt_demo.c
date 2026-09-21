// fmt_demo.c
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    printf(argv[1]);                 // SAI: người dùng điều khiển chuỗi định dạng
    printf("\n");
    printf("%s\n", argv[1]);         // ĐÚNG
    return 0;
}
