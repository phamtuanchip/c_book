// print_lines.c
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) { fprintf(stderr, "Cach dung: %s <file>\n", argv[0]); return 2; }

    FILE *f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 1; }

    char line[256];
    int n = 0;
    while (fgets(line, sizeof line, f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';        // bỏ ký tự xuống dòng (kể cả \r nếu file từ Windows)
        printf("%3d: %s\n", ++n, line);
    }
    if (ferror(f)) perror("doc file");             // phân biệt lỗi với hết file bình thường

    fclose(f);
    return 0;
}
