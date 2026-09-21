// stream_find.c
#include <stdio.h>
#include <string.h>

#define CHUNK 65536

/* Đếm số lần xuất hiện của pat trong f mà không đọc hết vào bộ nhớ.
   Giữ lại (len(pat)-1) byte cuối của khối trước để bắt mẫu bị cắt ở ranh giới. */
static long count_matches(FILE *f, const char *pat) {
    size_t m = strlen(pat);
    if (m == 0 || m > CHUNK) return -1;
    static char buf[2 * CHUNK];              // đủ chỗ cho phần đuôi giữ lại (< CHUNK) + một khối mới
    size_t keep = 0;                          // số byte đã giữ lại ở đầu buf
    long count = 0;
    size_t got;
    while ((got = fread(buf + keep, 1, CHUNK, f)) > 0) {
        size_t total = keep + got;
        for (size_t i = 0; i + m <= total; i++)
            if (memcmp(buf + i, pat, m) == 0) count++;      // memcmp: file có thể chứa byte 0
        keep = m - 1 < total ? m - 1 : total;
        memmove(buf, buf + total - keep, keep);             // giữ đuôi cho lần sau
        /* lưu ý: một lần khớp nằm trọn trong phần đuôi (< m) không thể bị đếm hai lần vì cần đủ m byte */
    }
    return count;
}

int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "cach dung: %s mau file\n", argv[0]); return 2; }
    FILE *f = fopen(argv[2], "rb");
    if (!f) { perror(argv[2]); return 1; }
    printf("%ld\n", count_matches(f, argv[1]));
    fclose(f);
    return 0;
}
