// copy_file.c
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Cach dung: %s <nguon> <dich>\n", argv[0]);
        return 2;
    }

    FILE *in  = fopen(argv[1], "rb");
    if (!in) { perror(argv[1]); return 1; }
    FILE *out = fopen(argv[2], "wb");
    if (!out) { perror(argv[2]); fclose(in); return 1; }

    unsigned char buf[8192];             // bộ đệm 8 KB
    size_t n;
    int rc = 0;
    while ((n = fread(buf, 1, sizeof buf, in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {          // ghi thiếu -> lỗi (đĩa đầy...)
            perror("fwrite");
            rc = 1;
            break;
        }
    }
    if (ferror(in)) { perror("fread"); rc = 1; }

    if (fclose(out) != 0) { perror("fclose"); rc = 1; }
    fclose(in);
    return rc;
}
