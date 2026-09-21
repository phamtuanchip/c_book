// tac.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    FILE *f = argc > 1 ? fopen(argv[1], "r") : stdin;
    if (!f) { perror(argv[1]); return 1; }

    char **lines = NULL;
    size_t n = 0, cap = 0;
    char buf[4096];
    while (fgets(buf, sizeof buf, f)) {
        if (n == cap) {
            size_t nc = cap ? cap * 2 : 64;
            char **t = realloc(lines, nc * sizeof *t);
            if (!t) { perror("realloc"); return 1; }
            lines = t; cap = nc;
        }
        lines[n] = malloc(strlen(buf) + 1);
        if (!lines[n]) { perror("malloc"); return 1; }
        strcpy(lines[n++], buf);
    }
    for (size_t i = n; i-- > 0; ) { fputs(lines[i], stdout); free(lines[i]); }
    free(lines);
    if (f != stdin) fclose(f);
    return 0;
}
