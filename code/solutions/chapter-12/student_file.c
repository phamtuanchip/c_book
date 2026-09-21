// student_file.c
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct { char name[32]; uint32_t age; double gpa; } Student;

static void put_u32(unsigned char *b, uint32_t v) { for (int i = 0; i < 4; i++) b[i] = (unsigned char)(v >> (8 * i)); }
static void put_u64(unsigned char *b, uint64_t v) { for (int i = 0; i < 8; i++) b[i] = (unsigned char)(v >> (8 * i)); }
static uint32_t get_u32(const unsigned char *b) { uint32_t v = 0; for (int i = 3; i >= 0; i--) v = (v << 8) | b[i]; return v; }
static uint64_t get_u64(const unsigned char *b) { uint64_t v = 0; for (int i = 7; i >= 0; i--) v = (v << 8) | b[i]; return v; }

static int save(const char *path, const Student *s, uint32_t n) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    unsigned char hdr[8];
    memcpy(hdr, "STU1", 4);
    put_u32(hdr + 4, n);
    int ok = fwrite(hdr, 1, 8, f) == 8;
    for (uint32_t i = 0; ok && i < n; i++) {
        unsigned char rec[44] = {0};                       // 32 tên + 4 tuổi + 8 gpa
        memcpy(rec, s[i].name, 31);                        // 31 ký tự + '\0' luôn có
        put_u32(rec + 32, s[i].age);
        uint64_t bits;
        memcpy(&bits, &s[i].gpa, sizeof bits);             // lấy bit pattern của double (IEEE 754)
        put_u64(rec + 36, bits);
        ok = fwrite(rec, 1, sizeof rec, f) == sizeof rec;
    }
    return (fclose(f) == 0 && ok) ? 0 : -1;
}

static int load(const char *path, Student *out, uint32_t cap, uint32_t *n) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    unsigned char hdr[8];
    if (fread(hdr, 1, 8, f) != 8 || memcmp(hdr, "STU1", 4) != 0) { fclose(f); return -1; }
    uint32_t count = get_u32(hdr + 4);
    if (count > cap) { fclose(f); return -1; }             // không tin số lượng do file khai
    for (uint32_t i = 0; i < count; i++) {
        unsigned char rec[44];
        if (fread(rec, 1, sizeof rec, f) != sizeof rec) { fclose(f); return -1; }
        memcpy(out[i].name, rec, 32);
        out[i].name[31] = '\0';
        out[i].age = get_u32(rec + 32);
        uint64_t bits = get_u64(rec + 36);
        memcpy(&out[i].gpa, &bits, sizeof bits);
    }
    fclose(f);
    *n = count;
    return 0;
}

int main(void) {
    Student in[2] = { {"An", 20, 3.5}, {"Binh", 21, 3.9} }, out[4];
    uint32_t n;
    if (save("stu.bin", in, 2) != 0 || load("stu.bin", out, 4, &n) != 0) return 1;
    for (uint32_t i = 0; i < n; i++) printf("%s %u %.2f\n", out[i].name, (unsigned)out[i].age, out[i].gpa);
    return 0;
}
