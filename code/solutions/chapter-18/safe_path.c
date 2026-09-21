// safe_path.c
#define _XOPEN_SOURCE 700              // để có realpath
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Ghép root + name, phân giải và bảo đảm kết quả vẫn nằm TRONG root. Trả 0 nếu an toàn. */
static int resolve_in_root(const char *root, const char *name, char *out, size_t cap) {
    char joined[4096], real_root[4096], real_path[4096];
    if (snprintf(joined, sizeof joined, "%s/%s", root, name) >= (int)sizeof joined) return -1;
    if (!realpath(root, real_root) || !realpath(joined, real_path)) return -1;   // cũng loại file không tồn tại
    size_t n = strlen(real_root);
    if (strncmp(real_path, real_root, n) != 0) return -1;
    if (real_path[n] != '/' && real_path[n] != '\0') return -1;                 // /srv/www-x không khớp /srv/www
    if (strlen(real_path) >= cap) return -1;
    strcpy(out, real_path);
    return 0;
}

int main(int argc, char **argv) {
    char out[4096];
    if (argc < 3) { fprintf(stderr, "cach dung: %s goc ten\n", argv[0]); return 2; }
    if (resolve_in_root(argv[1], argv[2], out, sizeof out) != 0) { puts("TU CHOI"); return 1; }
    printf("OK: %s\n", out);
    return 0;
}
