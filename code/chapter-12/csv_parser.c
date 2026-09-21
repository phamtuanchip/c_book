// csv_parser.c
#include <stdio.h>
#include <string.h>

// Tách `line` (SẼ BỊ SỬA) thành các trường, lưu con trỏ vào fields[]; trả về số trường.
size_t csv_split(char *line, char *fields[], size_t max_fields) {
    size_t n = 0;
    char *p = line;

    while (n < max_fields) {
        char *out;
        if (*p == '"') {                     // trường có nháy kép
            p++;
            out = p;
            char *w = p;                     // con trỏ ghi (nén "" thành ")
            while (*p) {
                if (*p == '"') {
                    if (p[1] == '"') { *w++ = '"'; p += 2; continue; }   // "" -> "
                    p++;                     // nháy đóng
                    break;
                }
                *w++ = *p++;
            }
            *w = '\0';
        } else {                             // trường thường
            out = p;
            while (*p && *p != ',') p++;
        }
        fields[n++] = out;

        if (*p == ',') { *p++ = '\0'; }      // kết thúc trường, sang trường kế
        else if (*p == '\0') break;          // hết dòng
        else { /* ký tự lạ sau nháy đóng: bỏ qua đến dấu phẩy */
            while (*p && *p != ',') p++;
            if (*p == ',') *p++ = '\0'; else break;
        }
    }
    return n;
}

int main(int argc, char *argv[]) {
    FILE *f = argc > 1 ? fopen(argv[1], "r") : stdin;
    if (!f) { perror("fopen"); return 1; }

    char line[1024];
    while (fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = '\0';
        char *fields[16];
        size_t n = csv_split(line, fields, 16);
        printf("%zu truong:", n);
        for (size_t i = 0; i < n; i++) printf(" [%s]", fields[i]);
        printf("\n");
    }
    if (f != stdin) fclose(f);
    return 0;
}
