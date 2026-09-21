// write_read.c
#include <stdio.h>
#include <string.h>

int main(void) {
    FILE *f = fopen("lines.txt", "w");                    // "w": xóa nội dung cũ nếu file đã có
    if (!f) { perror("lines.txt"); return 1; }
    for (int i = 1; i <= 10; i++) fprintf(f, "dong so %d\n", i);
    if (fclose(f) != 0) { perror("fclose"); return 1; }

    f = fopen("lines.txt", "r");
    if (!f) { perror("lines.txt"); return 1; }
    char line[128];
    int n = 0;
    while (fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = '\0';
        printf("%2d: %s\n", ++n, line);
    }
    fclose(f);
    return 0;
}
