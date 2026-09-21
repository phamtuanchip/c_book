// slurp.c
#include <stdio.h>
#include <stdlib.h>

/* Đọc toàn bộ f vào bộ đệm tăng dần. Trả bộ đệm (kèm '\0'), *len là số byte; NULL nếu lỗi. */
static char *slurp(FILE *f, size_t *len) {
    size_t cap = 4096, n = 0;
    char *buf = malloc(cap);
    if (!buf) return NULL;
    size_t got;
    while ((got = fread(buf + n, 1, cap - n - 1, f)) > 0) {
        n += got;
        if (cap - n < 2) {                          // sắp đầy: nhân đôi
            char *t = realloc(buf, cap * 2);
            if (!t) { free(buf); return NULL; }
            buf = t;
            cap *= 2;
        }
    }
    if (ferror(f)) { free(buf); return NULL; }
    buf[n] = '\0';
    if (len) *len = n;
    return buf;
}

int main(int argc, char **argv) {
    FILE *f = argc > 1 ? fopen(argv[1], "rb") : stdin;
    if (!f) { perror("fopen"); return 1; }
    size_t n;
    char *data = slurp(f, &n);
    if (!data) { fprintf(stderr, "loi doc\n"); return 1; }
    long lines = 0;
    for (size_t i = 0; i < n; i++) if (data[i] == '\n') lines++;
    printf("%zu byte, %ld dong\n", n, lines);
    free(data);
    if (f != stdin) fclose(f);
    return 0;
}
