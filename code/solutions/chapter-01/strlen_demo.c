// strlen_demo.c
#include <stdio.h>
#include <string.h>

static size_t my_strlen(const char *s) {
    size_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

int main(void) {
    char line[256];
    printf("Nhap mot dong: ");
    if (fgets(line, sizeof line, stdin) == NULL) return 1;
    line[strcspn(line, "\r\n")] = '\0';

    size_t a = strlen(line);
    size_t b = my_strlen(line);
    printf("strlen    = %zu\n", a);
    printf("my_strlen = %zu\n", b);
    printf("%s\n", a == b ? "Hai ket qua giong nhau" : "KHAC NHAU (co loi!)");
    return 0;
}
