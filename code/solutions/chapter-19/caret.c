// caret.c
#include <stdio.h>

/* In dòng nguồn rồi một dòng có dấu ^ dưới cột pos (0-based). */
void show_error(const char *line, size_t pos, const char *msg) {
    printf("%s\n", line);
    for (size_t i = 0; i < pos; i++) putchar(line[i] == '\t' ? '\t' : ' ');   // giữ tab để thẳng cột
    printf("^ %s\n", msg);
}

int main(void) {
    show_error("x = 2 +", 7, "can mot bieu thuc");
    return 0;
}
