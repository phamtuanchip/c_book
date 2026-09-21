// mini_grep.c
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 3) { fprintf(stderr, "Cach dung: %s <mau> <file>\n", argv[0]); return 2; }
    FILE *f = fopen(argv[2], "r");
    if (!f) { perror(argv[2]); return 2; }

    char line[4096];
    unsigned long lineno = 0;
    int found = 0;
    while (fgets(line, sizeof line, f)) {
        lineno++;
        if (strstr(line, argv[1])) {
            printf("%s:%lu: %s", argv[2], lineno, line);
            found = 1;
        }
    }
    fclose(f);
    return found ? 0 : 1;             // giống grep: 0 nếu tìm thấy, 1 nếu không
}
