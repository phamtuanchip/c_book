// split_line.c
#include <stdio.h>
#include <string.h>

// Tách line (SẼ BỊ SỬA) theo dấu phẩy; fields[i] trỏ vào các phần của line. Trả số trường.
static int split(char *line, char *fields[], int max) {
    int n = 0;
    char *p = line;
    while (n < max) {
        fields[n++] = p;
        char *comma = strchr(p, ',');
        if (!comma) break;
        *comma = '\0';
        p = comma + 1;
    }
    return n;
}

int main(void) {
    char line[] = "an,binh,,cuong";
    char *f[8];
    int n = split(line, f, 8);
    for (int i = 0; i < n; i++) printf("[%s]", f[i]);          // [an][binh][][cuong]
    printf("\n");
    return 0;
}
