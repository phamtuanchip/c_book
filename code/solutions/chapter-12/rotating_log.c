// rotating_log.c
#include <stdio.h>
#include <stdlib.h>

#define MAX_BYTES (1024L * 1024L)

/* Ghi một dòng vào path; nếu file đã >= MAX_BYTES thì đổi tên thành path.1 và bắt đầu file mới. */
static int log_line(const char *path, const char *line) {
    FILE *f = fopen(path, "a");
    if (!f) return -1;
    long size = ftell(f);                                   // "a": vị trí hiện tại là cuối file (trên hầu hết hệ thống)
    if (size < 0) { fclose(f); return -1; }
    if (size >= MAX_BYTES) {
        fclose(f);
        char old[512];
        snprintf(old, sizeof old, "%s.1", path);
        remove(old);                                        // rename đè có thể lỗi trên Windows nếu đích đã tồn tại
        if (rename(path, old) != 0) return -1;
        f = fopen(path, "a");
        if (!f) return -1;
    }
    int ok = fprintf(f, "%s\n", line) >= 0;
    return (fclose(f) == 0 && ok) ? 0 : -1;
}

int main(void) {
    for (int i = 0; i < 5; i++) {
        char msg[64];
        snprintf(msg, sizeof msg, "su kien %d", i);
        if (log_line("app.log", msg) != 0) { perror("log"); return 1; }
    }
    return 0;
}
