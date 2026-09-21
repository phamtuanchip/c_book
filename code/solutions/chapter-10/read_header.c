// read_header.c
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int read_u16_le(FILE *f, uint16_t *v) {
    unsigned char b[2];
    if (fread(b, 1, 2, f) != 2) return -1;
    *v = (uint16_t)(b[0] | (b[1] << 8));
    return 0;
}
static int read_u32_le(FILE *f, uint32_t *v) {
    unsigned char b[4];
    if (fread(b, 1, 4, f) != 4) return -1;
    *v = (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "cach dung: %s file\n", argv[0]); return 2; }
    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror(argv[1]); return 1; }

    char magic[4];
    uint16_t version;
    uint32_t count;
    if (fread(magic, 1, 4, f) != 4 || memcmp(magic, "CBK1", 4) != 0 ||
        read_u16_le(f, &version) != 0 || read_u32_le(f, &count) != 0) {
        fprintf(stderr, "header khong hop le\n");
        fclose(f);
        return 1;
    }
    printf("phien ban %u, %u ban ghi\n", (unsigned)version, (unsigned)count);
    fclose(f);
    return 0;
}
