// merge_files.c
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 3) { fprintf(stderr, "Cach dung: %s <dich> <nguon>...\n", argv[0]); return 2; }

    FILE *out = fopen(argv[1], "wb");
    if (!out) { perror(argv[1]); return 1; }

    int rc = 0;
    for (int i = 2; i < argc; i++) {
        FILE *in = fopen(argv[i], "rb");
        if (!in) { perror(argv[i]); rc = 1; continue; }      // bỏ qua file lỗi, vẫn xử lý file khác
        char buf[8192];
        size_t n;
        while ((n = fread(buf, 1, sizeof buf, in)) > 0) {
            if (fwrite(buf, 1, n, out) != n) { perror("fwrite"); rc = 1; break; }
        }
        fclose(in);
    }
    if (fclose(out) != 0) { perror("fclose"); rc = 1; }
    return rc;
}
