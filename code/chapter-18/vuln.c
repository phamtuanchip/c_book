// vuln.c
#include <stdio.h>
#include <string.h>

void greet(const char *name) {
    char buf[16];
    strcpy(buf, name);                   // KHÔNG kiểm tra độ dài
    printf("Xin chao, %s\n", buf);
}

int main(int argc, char *argv[]) {
    if (argc > 1) greet(argv[1]);
    return 0;
}
