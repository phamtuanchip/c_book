// bin_convert.c
#include <stdio.h>
#include <stdint.h>

// In n dưới dạng 32 bit nhị phân, nhóm mỗi 8 bit cho dễ đọc
void print_binary(uint32_t n) {
    for (int i = 31; i >= 0; i--) {
        putchar((n >> i) & 1 ? '1' : '0');   // dịch phải i bit, lấy bit thấp nhất
        if (i % 8 == 0 && i != 0) putchar(' ');
    }
    putchar('\n');
}

int main(void) {
    print_binary(13);          // 00000000 00000000 00000000 00001101
    print_binary(255);
    print_binary((uint32_t)-5);   // thấy biểu diễn bù hai của -5
    return 0;
}
