// generic_print.c
#include <stdio.h>

static void print_int(int x)          { printf("int: %d\n", x); }
static void print_long(long x)        { printf("long: %ld\n", x); }
static void print_double(double x)    { printf("double: %g\n", x); }
static void print_char(char x)        { printf("char: %c\n", x); }
static void print_str(const char *x)  { printf("string: %s\n", x); }

/* Chọn HÀM theo kiểu rồi mới gọi: mỗi nhánh chỉ là tên hàm nên không có cảnh báo định dạng ở nhánh không được chọn */
#define PRINT(x) _Generic((x),   \
    int:          print_int,     \
    long:         print_long,    \
    double:       print_double,  \
    char:         print_char,    \
    char *:       print_str,     \
    const char *: print_str)(x)

int main(void) {
    PRINT(42);
    PRINT(3.5);
    PRINT('A');                         // lưu ý: 'A' có kiểu int trong C, nên in "int: 65"
    PRINT("xin chao");                  // "xin chao" phân rã thành char * trong _Generic của gcc/clang
    return 0;
}
