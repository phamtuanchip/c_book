// os_name.c
#include <stdio.h>

int main(void) {
#if defined(_WIN32)
    puts("Windows");
#elif defined(__APPLE__)
    puts("macOS");
#elif defined(__linux__)
    puts("Linux");
#else
    puts("khong ro");
#endif
    return 0;
}
