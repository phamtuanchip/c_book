# Chương 12 — Lời giải bài tập

## Bài 1: ghi 10 dòng rồi đọc lại

```c
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
```

Mở `"w"` lần thứ hai (ví dụ chạy lại chương trình mà chỉ ghi 3 dòng) sẽ **xóa sạch** nội dung cũ; muốn nối thêm phải dùng `"a"`.

## Bài 2: `mycat` và `mywc`

`mywc` đã có ở mục 12.8 (`wc_lite.c`). `mycat` hỗ trợ nhiều file và stdin:

```c
// mycat.c
#include <stdio.h>

static int cat_stream(FILE *in, const char *name) {
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, in)) > 0)
        if (fwrite(buf, 1, n, stdout) != n) { perror("stdout"); return 1; }
    if (ferror(in)) { perror(name); return 1; }
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 1) return cat_stream(stdin, "stdin");        // không có đối số: đọc từ stdin
    int rc = 0;
    for (int i = 1; i < argc; i++) {
        FILE *f = fopen(argv[i], "rb");
        if (!f) { perror(argv[i]); rc = 1; continue; }        // báo lỗi rồi tiếp tục các file khác
        if (cat_stream(f, argv[i])) rc = 1;
        fclose(f);
    }
    return rc;
}
```

## Bài 3: sao chép file và xác minh

`copy_file.c` (mục 12.4). Kiểm chứng bản sao giống hệt: `cmp a.bin b.bin && echo GIONG` (Linux/macOS) hoặc `fc /b a.bin b.bin` (Windows CMD). Kiểm thử với file rỗng, file 1 byte, file đúng 8192 byte và 8193 byte (biên của bộ đệm), và file nhị phân có byte `0x00` và `0x1A`.

## Bài 4: ghép nhiều file

Xem `merge_files.c` (mục 12.8): mở từng file nguồn, báo lỗi và tiếp tục nếu một file không mở được, trả mã thoát khác 0 nếu có lỗi bất kỳ.

## Bài 5: điểm trung bình từ CSV

```c
// csv_scores.c
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    FILE *f = fopen(argc > 1 ? argv[1] : "scores.csv", "r");
    if (!f) { perror("scores.csv"); return 1; }

    char line[256], best_name[64] = "";
    double sum = 0, best = -1;
    int n = 0, lineno = 0, bad = 0;

    while (fgets(line, sizeof line, f)) {
        lineno++;
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;                                // bỏ dòng trống

        char *comma = strrchr(line, ',');                             // dấu phẩy cuối: tên có thể chứa dấu phẩy
        if (!comma) { fprintf(stderr, "dong %d: thieu dau phay\n", lineno); bad++; continue; }
        *comma = '\0';

        char *end;
        errno = 0;
        double score = strtod(comma + 1, &end);
        if (end == comma + 1 || *end != '\0' || errno == ERANGE || score < 0 || score > 10) {
            fprintf(stderr, "dong %d: diem khong hop le\n", lineno);
            bad++;
            continue;
        }
        sum += score;
        n++;
        if (score > best) { best = score; snprintf(best_name, sizeof best_name, "%s", line); }
    }
    fclose(f);

    if (n == 0) { fprintf(stderr, "khong co du lieu hop le\n"); return 1; }
    printf("%d hop le (%d loi), diem trung binh = %.2f, cao nhat: %s (%.1f)\n", n, bad, sum / n, best_name, best);
    return 0;
}
```

## Bài 6: lưu/đọc struct nhị phân theo định dạng tự quy định

Định dạng: magic `"STU1"` (4 byte), số bản ghi `uint32` little-endian, rồi mỗi bản ghi: `uint32` tuổi, `double` GPA lưu bằng **bit pattern** `uint64` little-endian, tên độ dài cố định 32 byte (đệm `\0`).

```c
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
```

File tạo ra giống hệt trên mọi máy (khác endianness, khác padding) — đó là mục tiêu của **tuần tự hóa từng trường**.

## Bài 7: tìm chuỗi trong file lớn (đúng ở ranh giới khối)

```c
// stream_find.c
#include <stdio.h>
#include <string.h>

#define CHUNK 65536

/* Đếm số lần xuất hiện của pat trong f mà không đọc hết vào bộ nhớ.
   Giữ lại (len(pat)-1) byte cuối của khối trước để bắt mẫu bị cắt ở ranh giới. */
static long count_matches(FILE *f, const char *pat) {
    size_t m = strlen(pat);
    if (m == 0 || m > CHUNK) return -1;
    static char buf[2 * CHUNK];              // đủ chỗ cho phần đuôi giữ lại (< CHUNK) + một khối mới
    size_t keep = 0;                          // số byte đã giữ lại ở đầu buf
    long count = 0;
    size_t got;
    while ((got = fread(buf + keep, 1, CHUNK, f)) > 0) {
        size_t total = keep + got;
        for (size_t i = 0; i + m <= total; i++)
            if (memcmp(buf + i, pat, m) == 0) count++;      // memcmp: file có thể chứa byte 0
        keep = m - 1 < total ? m - 1 : total;
        memmove(buf, buf + total - keep, keep);             // giữ đuôi cho lần sau
        /* lưu ý: một lần khớp nằm trọn trong phần đuôi (< m) không thể bị đếm hai lần vì cần đủ m byte */
    }
    return count;
}

int main(int argc, char **argv) {
    if (argc != 3) { fprintf(stderr, "cach dung: %s mau file\n", argv[0]); return 2; }
    FILE *f = fopen(argv[2], "rb");
    if (!f) { perror(argv[2]); return 1; }
    printf("%ld\n", count_matches(f, argv[1]));
    fclose(f);
    return 0;
}
```

Sai lầm cần tránh: `buf` phải rộng hơn `keep + CHUNK`; phần đuôi giữ lại **ngắn hơn** mẫu (`m − 1`) nên không thể chứa trọn một lần khớp, do đó không đếm trùng.

## Bài 8: đảo thứ tự dòng của file

Cách đơn giản: đọc các dòng vào một mảng con trỏ động (hoặc danh sách liên kết), rồi in ngược.

```c
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
```

(Dòng dài hơn 4095 ký tự sẽ bị tách thành nhiều "dòng" — dùng `read_line` ở mục 12.3 nếu cần chính xác.) Với file rất lớn, cách tốt hơn là `fseek` từ cuối file lùi dần.

## Bài 9: logger xoay vòng

```c
// rotating_log.c
#include <stdio.h>
#include <stdlib.h>

#define MAX_BYTES (1024L * 1024L)

/* Ghi một dòng vào path; nếu file đã >= MAX_BYTES thì đổi tên thành path.1 và bắt đầu file mới. */
static int log_line(const char *path, const char *line) {
    FILE *f = fopen(path, "a");
    if (!f) return -1;
    long size = ftell(f);                                   // "a": vị trí hiện tại là cuối file (trên hầu hết hệ thống)
    if (size < 0) { fclose(f); return -1; }
    if (size >= MAX_BYTES) {
        fclose(f);
        char old[512];
        snprintf(old, sizeof old, "%s.1", path);
        remove(old);                                        // rename đè có thể lỗi trên Windows nếu đích đã tồn tại
        if (rename(path, old) != 0) return -1;
        f = fopen(path, "a");
        if (!f) return -1;
    }
    int ok = fprintf(f, "%s\n", line) >= 0;
    return (fclose(f) == 0 && ok) ? 0 : -1;
}

int main(void) {
    for (int i = 0; i < 5; i++) {
        char msg[64];
        snprintf(msg, sizeof msg, "su kien %d", i);
        if (log_line("app.log", msg) != 0) { perror("log"); return 1; }
    }
    return 0;
}
```

Mở/đóng file mỗi dòng đơn giản nhưng chậm; bản thực tế giữ `FILE *` mở và tự đếm số byte đã ghi. Với nhiều tiến trình cùng ghi cần khóa file hoặc dùng `syslog`.
