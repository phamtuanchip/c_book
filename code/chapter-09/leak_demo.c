// leak_demo.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void process(void) {
    char *buf = malloc(1024);
    if (!buf) return;
    strcpy(buf, "hello");
    printf("%s\n", buf);
    // quên free(buf)  -> rò rỉ 1024 byte mỗi lần gọi
}

int main(void) {
    for (int i = 0; i < 1000; i++) process();    // rò rỉ ~1 MB
    return 0;
}
