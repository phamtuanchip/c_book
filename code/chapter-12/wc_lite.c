// wc_lite.c
#include <stdio.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
    FILE *f = argc > 1 ? fopen(argv[1], "rb") : stdin;
    if (!f) { perror(argv[1]); return 1; }

    unsigned long lines = 0, words = 0, bytes = 0;
    int in_word = 0, c;
    while ((c = fgetc(f)) != EOF) {
        bytes++;
        if (c == '\n') lines++;
        if (isspace(c)) in_word = 0;
        else if (!in_word) { in_word = 1; words++; }
    }
    printf("%lu %lu %lu\n", lines, words, bytes);
    if (f != stdin) fclose(f);
    return 0;
}
