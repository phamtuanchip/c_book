#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "Usage: %s SOURCE DEST\n", argv[0]); return 1; }
    FILE *in = fopen(argv[1], "rb");
    if (!in) { perror("fopen source"); return 1; }
    FILE *out = fopen(argv[2], "wb");
    if (!out) { perror("fopen dest"); fclose(in); return 1; }
    char buf[8192]; size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) { perror("fwrite"); fclose(in); fclose(out); return 1; }
    }
    if (ferror(in)) { perror("fread"); }
    fclose(in); fclose(out);
    return 0;
}
