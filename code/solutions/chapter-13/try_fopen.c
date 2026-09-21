// try_fopen.c
#include <errno.h>
#include <stdio.h>
#include <string.h>

static FILE *safe_fopen(const char *path, const char *mode, char *err, size_t cap) {
    FILE *f = fopen(path, mode);
    if (!f) {
        int saved = errno;
        snprintf(err, cap, "khong mo duoc '%s' (%s): %s", path, mode, strerror(saved));
        errno = saved;
        return NULL;
    }
    err[0] = '\0';
    return f;
}

int main(void) {
    const char *paths[] = { "khong_ton_tai.txt", ".", "/etc/shadow" };   // không có file / là thư mục / không có quyền
    char err[200];
    for (size_t i = 0; i < 3; i++) {
        FILE *f = safe_fopen(paths[i], "r", err, sizeof err);
        if (!f) printf("loi: %s\n", err);
        else { printf("mo duoc %s\n", paths[i]); fclose(f); }
    }
    return 0;
}
