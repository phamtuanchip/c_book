// ovf.c
#include <string.h>

int main(int argc, char **argv) {
    char buf[8];
    if (argc > 1) strcpy(buf, argv[1]);          // LỖI: không kiểm tra độ dài
    return buf[0];
}
