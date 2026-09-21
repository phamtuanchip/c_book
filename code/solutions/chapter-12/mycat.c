// mycat.c
#include <stdio.h>

static int cat_stream(FILE *in, const char *name) {
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, in)) > 0)
        if (fwrite(buf, 1, n, stdout) != n) { perror("stdout"); return 1; }
    if (ferror(in)) { perror(name); return 1; }
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) return cat_stream(stdin, "stdin");        // không có đối số: đọc từ stdin
    int rc = 0;
    for (int i = 1; i < argc; i++) {
        FILE *f = fopen(argv[i], "rb");
        if (!f) { perror(argv[i]); rc = 1; continue; }        // báo lỗi rồi tiếp tục các file khác
        if (cat_stream(f, argv[i])) rc = 1;
        fclose(f);
    }
    return rc;
}
