// sum_file.c
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { SUM_OK = 0, SUM_ERR_IO, SUM_ERR_PARSE, SUM_ERR_OVERFLOW } SumError;

static const char *sum_error_str(SumError e) {
    switch (e) {
        case SUM_OK:           return "thanh cong";
        case SUM_ERR_IO:       return "loi doc file";
        case SUM_ERR_PARSE:    return "dong khong phai so nguyen";
        case SUM_ERR_OVERFLOW: return "tong bi tran";
    }
    return "?";
}

// Tính tổng các dòng trong file. *bad_line trả về số dòng lỗi (nếu có).
static SumError sum_file(const char *path, long *out_sum, unsigned *bad_line) {
    FILE *f = fopen(path, "r");
    if (!f) return SUM_ERR_IO;

    SumError rc = SUM_OK;
    long sum = 0;
    char line[128];
    unsigned lineno = 0;

    while (fgets(line, sizeof line, f)) {
        lineno++;
        char *end;
        errno = 0;
        long v = strtol(line, &end, 10);
        if (end == line || (*end != '\n' && *end != '\0') || errno == ERANGE) {
            rc = SUM_ERR_PARSE; *bad_line = lineno; goto done;
        }
        if ((v > 0 && sum > LONG_MAX - v) || (v < 0 && sum < LONG_MIN - v)) {
            rc = SUM_ERR_OVERFLOW; *bad_line = lineno; goto done;
        }
        sum += v;
    }
    if (ferror(f)) rc = SUM_ERR_IO;
    else *out_sum = sum;

done:
    fclose(f);
    return rc;
}

int main(int argc, char *argv[]) {
    if (argc != 2) { fprintf(stderr, "Cach dung: %s <file>\n", argv[0]); return 2; }

    long sum = 0;
    unsigned bad = 0;
    SumError e = sum_file(argv[1], &sum, &bad);
    if (e != SUM_OK) {
        if (e == SUM_ERR_IO)
            fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], strerror(errno));
        else
            fprintf(stderr, "%s:%u: %s\n", argv[1], bad, sum_error_str(e));
        return 1;
    }
    printf("tong = %ld\n", sum);
    return 0;
}
