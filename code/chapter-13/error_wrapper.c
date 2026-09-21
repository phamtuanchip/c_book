#include <stdio.h>
#include <errno.h>
#include <string.h>

int open_file_read(const char *path, FILE **out) {
    FILE *f = fopen(path, "r");
    if (!f) {
        int e = errno;
        fprintf(stderr, "open_file_read: failed to open '%s': %s\n", path, strerror(e));
        return e;
    }
    *out = f;
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "Usage: %s file\n", argv[0]); return 1; }
    FILE *f;
    int err = open_file_read(argv[1], &f);
    if (err) return 1;
    printf("Opened %s successfully\n", argv[1]);
    fclose(f);
    return 0;
}
