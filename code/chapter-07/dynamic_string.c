#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simple dynamic string builder */
int main(void) {
    size_t cap = 16;
    size_t len = 0;
    char *s = malloc(cap);
    if (!s) { perror("malloc"); return 1; }
    s[0] = '\0';

    printf("Nhap cac dong (ket thuc Ctrl+D / Ctrl+Z):\n");
    char line[128];
    while (fgets(line, sizeof(line), stdin)) {
        size_t add = strlen(line);
        if (len + add + 1 > cap) {
            while (len + add + 1 > cap) cap *= 2;
            char *tmp = realloc(s, cap);
            if (!tmp) { free(s); perror("realloc"); return 1; }
            s = tmp;
        }
        memcpy(s + len, line, add + 1);
        len += add;
    }

    printf("Full content (length=%zu):\n%s\n", len, s);
    free(s);
    return 0;
}
