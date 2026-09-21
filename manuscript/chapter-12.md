# Chương 12 — File I/O & thao tác hệ thống

## Mục tiêu chương

- Hiểu khái niệm **luồng (stream)**, `FILE *`, và ba luồng chuẩn `stdin`, `stdout`, `stderr`.
- Đọc/ghi file **văn bản** (`fopen`, `fgets`, `fprintf`, `fscanf`) và file **nhị phân** (`fread`, `fwrite`).
- Hiểu **bộ đệm (buffering)**, `fflush`, `fclose`, và khi nào dữ liệu thật sự được ghi xuống đĩa.
- Định vị trong file (`fseek`, `ftell`, `rewind`), xác định kích thước file.
- Xử lý lỗi I/O đúng cách: kiểm tra giá trị trả về, `feof`/`ferror`, `perror`, `errno`.
- Viết các chương trình thực tế: sao chép file, đọc CSV, đếm từ, tìm chuỗi trong file lớn.
- Biết sơ lược về thao tác hệ thống: thư mục, biến môi trường, `mmap` và tính di động.

## 12.1. Luồng (stream) và `FILE *`

Thư viện chuẩn C trừu tượng hóa mọi thao tác nhập/xuất (file, bàn phím, màn hình, ống dẫn) thành **luồng byte**. Bạn thao tác với luồng qua một con trỏ kiểu `FILE *` (định nghĩa trong `<stdio.h>`); bên trong nó lưu vị trí đọc/ghi, bộ đệm, cờ lỗi/EOF.

Ba luồng có sẵn khi chương trình khởi động:

| Luồng | Vai trò | Mặc định nối tới |
|---|---|---|
| `stdin` | đầu vào chuẩn | bàn phím |
| `stdout` | đầu ra chuẩn | màn hình |
| `stderr` | lỗi chuẩn (thông báo lỗi) | màn hình |

Các hàm bạn đã dùng thực ra là dạng rút gọn: `printf(...)` ≡ `fprintf(stdout, ...)`, `scanf(...)` ≡ `fscanf(stdin, ...)`, `getchar()` ≡ `fgetc(stdin)`. Nhờ vậy, mọi hàm làm việc với `stdin/stdout` cũng làm việc với file.

**Chuyển hướng (redirection)** ở shell nối các luồng này với file mà không cần sửa mã:

```bash
./prog < input.txt > output.txt 2> errors.txt
```

Vì vậy hãy in thông báo lỗi ra `stderr` để chúng không lẫn vào `output.txt`.

## 12.2. Mở và đóng file

```c
FILE *fopen(const char *path, const char *mode);
int   fclose(FILE *stream);
```

`fopen` trả về `FILE *` nếu thành công, **`NULL` nếu thất bại** (file không tồn tại, không có quyền, ...). Luôn kiểm tra.

### Chế độ mở (mode)

| Mode | Ý nghĩa | File chưa tồn tại | File đã tồn tại |
|---|---|---|---|
| `"r"` | đọc | **lỗi** (trả NULL) | đọc từ đầu |
| `"w"` | ghi | tạo mới | **xóa sạch nội dung** rồi ghi |
| `"a"` | ghi nối thêm (append) | tạo mới | ghi vào **cuối** |
| `"r+"` | đọc và ghi | lỗi | đọc/ghi từ đầu, không xóa |
| `"w+"` | đọc và ghi | tạo mới | **xóa sạch** |
| `"a+"` | đọc và nối thêm | tạo mới | ghi luôn vào cuối |

Thêm **`b`** cho chế độ **nhị phân** (`"rb"`, `"wb"`). Trên **Windows**, chế độ văn bản dịch `\n` ↔ `\r\n` và coi byte `0x1A` là hết file; trên Linux/macOS không có khác biệt. Vì vậy: **luôn dùng `"rb"`/`"wb"` cho dữ liệu nhị phân** để mã chạy đúng trên mọi nền tảng.

C11 thêm chế độ `"x"` (ví dụ `"wx"`): tạo file **và báo lỗi nếu đã tồn tại** — tránh ghi đè vô ý.

### Mẫu chuẩn: mở, dùng, đóng, kiểm tra

```c
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(void) {
    FILE *f = fopen("data.txt", "r");
    if (f == NULL) {
        fprintf(stderr, "Khong mo duoc data.txt: %s\n", strerror(errno));
        // hoặc: perror("fopen");
        return 1;
    }

    /* ... đọc/ghi ... */

    if (fclose(f) != 0) {           // fclose cũng có thể lỗi (ví dụ đĩa đầy khi xả bộ đệm cuối)
        perror("fclose");
        return 1;
    }
    return 0;
}
```

- `errno` (trong `<errno.h>`) chứa mã lỗi của lần gọi hệ thống gần nhất bị lỗi. `strerror(errno)` đổi nó thành thông báo đọc được; `perror("tiền tố")` in luôn ra `stderr` dạng `tiền tố: thông báo`.
- **Phải `fclose` mọi file đã mở**: giải phóng tài nguyên, và quan trọng hơn, **xả (flush) bộ đệm** để dữ liệu ghi thật sự ra đĩa. Mỗi tiến trình bị giới hạn số file mở đồng thời (thường ~1024), nên quên `fclose` trong vòng lặp sẽ sớm gặp lỗi `Too many open files`.

## 12.3. Đọc và ghi file văn bản

### Ghi

```c
int  fputc(int c, FILE *f);                         // một ký tự
int  fputs(const char *s, FILE *f);                 // một chuỗi (KHÔNG thêm '\n')
int  fprintf(FILE *f, const char *fmt, ...);        // có định dạng
```

```c
// write_text.c
#include <stdio.h>

int main(void) {
    FILE *f = fopen("scores.txt", "w");
    if (!f) { perror("fopen"); return 1; }

    const char *names[] = {"An", "Binh", "Cuong"};
    int scores[] = {8, 9, 7};
    for (int i = 0; i < 3; i++) {
        if (fprintf(f, "%s,%d\n", names[i], scores[i]) < 0) {   // fprintf trả số âm khi lỗi
            perror("fprintf");
            fclose(f);
            return 1;
        }
    }
    return fclose(f) == 0 ? 0 : 1;
}
```

### Đọc theo dòng: `fgets`

```c
char *fgets(char *buf, int size, FILE *f);
```

Đọc tối đa `size - 1` ký tự (dừng sau `'\n'`), thêm `'\0'`. Trả `NULL` khi hết file hoặc lỗi. Đây là cách **an toàn và được khuyến nghị** để đọc văn bản.

```c
// print_lines.c
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) { fprintf(stderr, "Cach dung: %s <file>\n", argv[0]); return 2; }

    FILE *f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 1; }

    char line[256];
    int n = 0;
    while (fgets(line, sizeof line, f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';        // bỏ ký tự xuống dòng (kể cả \r nếu file từ Windows)
        printf("%3d: %s\n", ++n, line);
    }
    if (ferror(f)) perror("doc file");             // phân biệt lỗi với hết file bình thường

    fclose(f);
    return 0;
}
```

**Vấn đề với dòng dài hơn bộ đệm:** nếu dòng dài hơn 255 ký tự, `fgets` chỉ trả về một phần, phần còn lại được trả ở lần gọi sau (dòng bị "tách đôi"). Nếu bạn cần dòng có độ dài bất kỳ, dùng `getline` (POSIX) hoặc tự viết hàm mở rộng bộ đệm:

```c
// Đọc một dòng dài tùy ý. Trả về chuỗi cấp phát bằng malloc (người gọi free), hoặc NULL nếu hết file/lỗi.
char *read_line(FILE *f) {
    size_t cap = 128, len = 0;
    char *buf = malloc(cap);
    if (!buf) return NULL;

    int c;
    while ((c = fgetc(f)) != EOF && c != '\n') {
        if (len + 1 >= cap) {
            cap *= 2;
            char *tmp = realloc(buf, cap);
            if (!tmp) { free(buf); return NULL; }
            buf = tmp;
        }
        buf[len++] = (char)c;
    }
    if (c == EOF && len == 0) { free(buf); return NULL; }    // không còn dữ liệu
    buf[len] = '\0';
    return buf;
}
```

### Đọc có định dạng: `fscanf`

```c
int fscanf(FILE *f, const char *fmt, ...);
```

```c
int a, b;
while (fscanf(f, "%d %d", &a, &b) == 2) {        // == 2: cả hai giá trị đều đọc được
    printf("%d + %d = %d\n", a, b, a + b);
}
```

`fscanf` tiện nhưng khó xử lý lỗi (khi gặp dữ liệu sai, nó dừng và để nguyên đầu vào, dễ lặp vô hạn) nên với dữ liệu không tin cậy hãy **đọc dòng bằng `fgets` rồi dùng `sscanf`/`strtol`** để phân tích:

```c
char line[128];
while (fgets(line, sizeof line, f)) {
    char name[32];
    int score;
    if (sscanf(line, "%31[^,],%d", name, &score) == 2) {   // %31[^,]: đọc tới dấu phẩy, tối đa 31 ký tự
        printf("%s co diem %d\n", name, score);
    } else {
        fprintf(stderr, "dong khong hop le: %s", line);
    }
}
```

### Đọc từng ký tự: `fgetc`, `getc`

Giá trị trả về là `int`, không phải `char`, để phân biệt được `EOF` (thường là `-1`) với ký tự có giá trị 255:

```c
int c;
long count = 0;
while ((c = fgetc(f)) != EOF) {
    if (c == '\n') count++;
}
printf("%ld dong\n", count);
```

### Hết file (EOF) — hiểu đúng

Sai lầm phổ biến:

```c
while (!feof(f)) {            // SAI!
    fgets(line, sizeof line, f);
    process(line);            // lần cuối xử lý dữ liệu cũ vì feof chỉ đúng SAU khi đã thử đọc vượt cuối
}
```

`feof` chỉ trả về đúng **sau khi** một thao tác đọc thất bại vì hết file. Cách đúng là **kiểm tra giá trị trả về của hàm đọc** trong điều kiện vòng lặp (`while (fgets(...) != NULL)`); dùng `feof`/`ferror` **sau** vòng lặp để phân biệt hết file với lỗi.

## 12.4. Đọc và ghi nhị phân

```c
size_t fread (void *buf,       size_t size, size_t count, FILE *f);
size_t fwrite(const void *buf, size_t size, size_t count, FILE *f);
```

Đọc/ghi `count` phần tử, mỗi phần tử `size` byte; trả về **số phần tử** thực sự xử lý được (nhỏ hơn `count` nghĩa là hết file hoặc lỗi).

### Ví dụ: sao chép file (mọi loại)

```c
// copy_file.c
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Cach dung: %s <nguon> <dich>\n", argv[0]);
        return 2;
    }

    FILE *in  = fopen(argv[1], "rb");
    if (!in) { perror(argv[1]); return 1; }
    FILE *out = fopen(argv[2], "wb");
    if (!out) { perror(argv[2]); fclose(in); return 1; }

    unsigned char buf[8192];             // bộ đệm 8 KB
    size_t n;
    int rc = 0;
    while ((n = fread(buf, 1, sizeof buf, in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {          // ghi thiếu -> lỗi (đĩa đầy...)
            perror("fwrite");
            rc = 1;
            break;
        }
    }
    if (ferror(in)) { perror("fread"); rc = 1; }

    if (fclose(out) != 0) { perror("fclose"); rc = 1; }
    fclose(in);
    return rc;
}
```

Điểm đáng nhớ: dùng `size = 1` và `count = n` để biết chính xác **số byte** đã đọc; đọc theo khối (8 KB) hiệu quả hơn nhiều so với từng byte.

### Ghi/đọc mảng và struct nhị phân

```c
typedef struct { int id; double score; } Record;

Record recs[3] = { {1, 9.5}, {2, 8.0}, {3, 7.25} };

FILE *f = fopen("data.bin", "wb");
fwrite(recs, sizeof recs[0], 3, f);
fclose(f);

Record back[3];
f = fopen("data.bin", "rb");
size_t got = fread(back, sizeof back[0], 3, f);
fclose(f);
```

**Cảnh báo về tính di động:** cách này ghi **nguyên khối byte của struct**, gồm cả **padding**, kích thước kiểu và **thứ tự byte (endianness)** của máy hiện tại. File tạo ra trên máy này có thể **không đọc đúng** trên máy khác (khác endianness, khác `sizeof(long)`, khác padding). Với dữ liệu trao đổi giữa hệ thống, hãy **tuần tự hóa từng trường** với kiểu có kích thước cố định:

```c
#include <stdint.h>

// Ghi uint32_t theo thứ tự byte big-endian, không phụ thuộc máy
int write_u32_be(FILE *f, uint32_t v) {
    unsigned char b[4] = {
        (unsigned char)(v >> 24), (unsigned char)(v >> 16),
        (unsigned char)(v >> 8),  (unsigned char)v
    };
    return fwrite(b, 1, 4, f) == 4 ? 0 : -1;
}

int read_u32_be(FILE *f, uint32_t *out) {
    unsigned char b[4];
    if (fread(b, 1, 4, f) != 4) return -1;
    *out = ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 8) | b[3];
    return 0;
}
```

## 12.5. Di chuyển trong file

```c
int  fseek(FILE *f, long offset, int whence);   // whence: SEEK_SET (đầu), SEEK_CUR (hiện tại), SEEK_END (cuối)
long ftell(FILE *f);                            // vị trí hiện tại (byte)
void rewind(FILE *f);                           // về đầu file và xóa cờ lỗi
```

### Lấy kích thước file

```c
long file_size(FILE *f) {
    long cur = ftell(f);
    if (cur < 0) return -1;
    if (fseek(f, 0, SEEK_END) != 0) return -1;
    long size = ftell(f);
    fseek(f, cur, SEEK_SET);                    // trả về vị trí cũ
    return size;
}
```

### Đọc toàn bộ file vào bộ nhớ

```c
// Trả về bộ đệm cấp phát bằng malloc chứa toàn bộ file (kèm '\0' cuối); *out_len là số byte.
char *read_all(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long size = ftell(f);
    if (size < 0) { fclose(f); return NULL; }
    rewind(f);

    char *buf = malloc((size_t)size + 1);
    if (!buf) { fclose(f); return NULL; }

    size_t got = fread(buf, 1, (size_t)size, f);
    fclose(f);
    if (got != (size_t)size) { free(buf); return NULL; }

    buf[size] = '\0';
    if (out_len) *out_len = (size_t)size;
    return buf;
}
```

Với file lớn hơn RAM, **không** đọc hết vào bộ nhớ; hãy xử lý theo dòng/khối (mục 12.8). `long` trên Windows chỉ 32 bit nên `ftell` giới hạn 2 GB; với file lớn dùng `_fseeki64`/`fseeko`.

### Truy cập ngẫu nhiên: cập nhật một bản ghi

```c
// Đọc bản ghi thứ i và sửa nó, dùng file "r+b"
FILE *f = fopen("data.bin", "r+b");
fseek(f, (long)(i * sizeof(Record)), SEEK_SET);
Record r;
fread(&r, sizeof r, 1, f);
r.score += 1.0;
fseek(f, (long)(i * sizeof(Record)), SEEK_SET);   // BẮT BUỘC fseek trước khi chuyển giữa đọc và ghi
fwrite(&r, sizeof r, 1, f);
fclose(f);
```

## 12.6. Bộ đệm (buffering)

Gọi hệ thống để ghi từng byte rất chậm nên thư viện C dùng **bộ đệm**: dữ liệu ghi được gom vào bộ nhớ và chỉ chuyển cho hệ điều hành khi bộ đệm đầy, khi bạn `fflush`, hoặc khi `fclose`.

Có ba chế độ:

| Chế độ | Khi nào xả | Mặc định của |
|---|---|---|
| **Đệm đầy (fully buffered)** | khi bộ đệm đầy | file thường |
| **Đệm theo dòng (line buffered)** | khi gặp `'\n'` | `stdout` nối vào terminal |
| **Không đệm (unbuffered)** | ngay lập tức | `stderr` |

Hệ quả thực tế:

1. **`printf` không có `\n` có thể không hiện ngay** khi `stdout` là terminal (đệm theo dòng), và **không hiện ngay khi nối vào file/ống** (đệm đầy). Nếu chương trình crash, dòng chưa xả sẽ **mất**. Dùng `fflush(stdout)` khi cần hoặc in ra `stderr`.
2. Trước khi đọc từ bàn phím, để lời nhắc không có `\n` hiện ra kịp thời: `printf("Nhap: "); fflush(stdout);`. (Thực tế nhiều hệ thống tự xả `stdout` khi đọc `stdin`, nhưng đừng dựa vào đó.)
3. `fflush(f)` chỉ đẩy dữ liệu từ bộ đệm C cho hệ điều hành; hệ điều hành lại có **bộ đệm riêng** trước khi xuống đĩa. Để bảo đảm dữ liệu đã **thật sự nằm trên đĩa** (chống mất điện), cần `fsync(fileno(f))` (POSIX) hoặc `_commit` (Windows).

```c
setvbuf(f, NULL, _IOFBF, 65536);   // đặt bộ đệm 64 KB, đệm đầy (gọi ngay sau fopen, trước mọi thao tác khác)
setvbuf(stdout, NULL, _IONBF, 0);  // tắt bộ đệm stdout
```

**Ghi file an toàn** (không để file dở dang nếu chương trình bị dừng giữa chừng): ghi ra file tạm rồi `rename` đè lên file đích (thao tác `rename` là nguyên tử trên hệ POSIX):

```c
FILE *f = fopen("config.tmp", "w");
/* ... ghi ... */
fclose(f);
rename("config.tmp", "config.txt");
```

## 12.7. Xử lý lỗi I/O — tổng hợp

| Hàm | Dấu hiệu lỗi |
|---|---|
| `fopen` | trả `NULL` (đặt `errno`) |
| `fgets` | trả `NULL` (hết file hoặc lỗi — phân biệt bằng `feof`/`ferror`) |
| `fgetc`/`getchar` | trả `EOF` |
| `fread`/`fwrite` | trả về số phần tử **nhỏ hơn** yêu cầu |
| `fprintf`/`fputs` | trả số âm / `EOF` |
| `fscanf` | trả số mục đọc được ít hơn mong đợi (hoặc `EOF`) |
| `fseek` | trả khác 0 |
| `fclose` | trả khác 0 |

Mẫu hàm bọc để kiểm tra lỗi đồng nhất:

```c
FILE *xfopen(const char *path, const char *mode) {
    FILE *f = fopen(path, mode);
    if (!f) {
        fprintf(stderr, "loi: khong mo duoc '%s' (%s): %s\n", path, mode, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return f;
}
```

Những nguyên nhân thất bại thường gặp: sai đường dẫn (`ENOENT`), không có quyền (`EACCES`), thư mục thay vì file (`EISDIR`), đĩa đầy (`ENOSPC`), quá nhiều file mở (`EMFILE`). Chương 13 bàn kỹ hơn về chiến lược xử lý lỗi.

**Đường dẫn trên Windows:** dùng `/` hoặc `\\` (trong chuỗi C, dấu `\` phải viết `\\`): `"C:\\data\\a.txt"`. Đường dẫn tương đối tính từ **thư mục làm việc hiện tại** (nơi bạn chạy chương trình), không phải nơi chứa file `.exe`.

## 12.8. Ví dụ thực tế

### CSV đơn giản có trường trong dấu nháy kép

Định dạng CSV: các trường phân cách bằng dấu phẩy; trường chứa dấu phẩy hoặc xuống dòng được bao trong `"..."`, và dấu `"` bên trong viết thành `""`.

```c
// csv_parser.c — phân tích một dòng CSV vào mảng trường
#include <stdio.h>
#include <string.h>

// Tách `line` (SẼ BỊ SỬA) thành các trường, lưu con trỏ vào fields[]; trả về số trường.
size_t csv_split(char *line, char *fields[], size_t max_fields) {
    size_t n = 0;
    char *p = line;

    while (n < max_fields) {
        char *out;
        if (*p == '"') {                     // trường có nháy kép
            p++;
            out = p;
            char *w = p;                     // con trỏ ghi (nén "" thành ")
            while (*p) {
                if (*p == '"') {
                    if (p[1] == '"') { *w++ = '"'; p += 2; continue; }   // "" -> "
                    p++;                     // nháy đóng
                    break;
                }
                *w++ = *p++;
            }
            *w = '\0';
        } else {                             // trường thường
            out = p;
            while (*p && *p != ',') p++;
        }
        fields[n++] = out;

        if (*p == ',') { *p++ = '\0'; }      // kết thúc trường, sang trường kế
        else if (*p == '\0') break;          // hết dòng
        else { /* ký tự lạ sau nháy đóng: bỏ qua đến dấu phẩy */
            while (*p && *p != ',') p++;
            if (*p == ',') *p++ = '\0'; else break;
        }
    }
    return n;
}

int main(int argc, char *argv[]) {
    FILE *f = argc > 1 ? fopen(argv[1], "r") : stdin;
    if (!f) { perror("fopen"); return 1; }

    char line[1024];
    while (fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = '\0';
        char *fields[16];
        size_t n = csv_split(line, fields, 16);
        printf("%zu truong:", n);
        for (size_t i = 0; i < n; i++) printf(" [%s]", fields[i]);
        printf("\n");
    }
    if (f != stdin) fclose(f);
    return 0;
}
```

Ví dụ `an,"Nguyen, Van B","noi ""hay"""` cho ba trường: `an`, `Nguyen, Van B`, `noi "hay"`. Mã này cố ý gọn, đủ dạy nguyên lý; bộ phân tích CSV thực tế cần xử lý cả trường chứa xuống dòng và các trường hợp hỏng.

### Đếm dòng, từ, byte (giống `wc`)

```c
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
```

### Tìm chuỗi trong file lớn (streaming)

Đọc từng dòng, giữ bộ nhớ hằng số bất kể file lớn thế nào — giống `grep` đơn giản:

```c
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
```

Hạn chế: mẫu bị cắt đôi ở ranh giới hai lần đọc khi một dòng dài hơn 4095 ký tự sẽ bị bỏ sót. Muốn chắc chắn, dùng `getline` hoặc `read_line` ở trên.

### Ghép nhiều file (`cat`)

```c
// merge_files.c — nối các file đầu vào vào một file đầu ra
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 3) { fprintf(stderr, "Cach dung: %s <dich> <nguon>...\n", argv[0]); return 2; }

    FILE *out = fopen(argv[1], "wb");
    if (!out) { perror(argv[1]); return 1; }

    int rc = 0;
    for (int i = 2; i < argc; i++) {
        FILE *in = fopen(argv[i], "rb");
        if (!in) { perror(argv[i]); rc = 1; continue; }      // bỏ qua file lỗi, vẫn xử lý file khác
        char buf[8192];
        size_t n;
        while ((n = fread(buf, 1, sizeof buf, in)) > 0) {
            if (fwrite(buf, 1, n, out) != n) { perror("fwrite"); rc = 1; break; }
        }
        fclose(in);
    }
    if (fclose(out) != 0) { perror("fclose"); rc = 1; }
    return rc;
}
```

## 12.9. Thao tác hệ thống cơ bản

Thư viện chuẩn C có một số hàm thao tác file ở mức tên:

```c
remove("old.txt");                // xóa file (trả 0 nếu thành công)
rename("a.txt", "b.txt");         // đổi tên/di chuyển (trong cùng hệ thống file)
FILE *t = tmpfile();              // file tạm tự xóa khi đóng/thoát
```

### Tham số dòng lệnh và mã thoát

```c
int main(int argc, char *argv[])   // argc: số đối số, argv[0]: tên chương trình
```

Dùng `return`/`exit` với `EXIT_SUCCESS` (0) hoặc `EXIT_FAILURE` (1) hoặc mã riêng. Quy ước của công cụ dòng lệnh: `0` thành công; `1` lỗi chung; `2` dùng sai cú pháp.

### Biến môi trường

```c
#include <stdlib.h>
const char *home = getenv("HOME");         // NULL nếu không có (Windows: "USERPROFILE")
```

### Chạy lệnh ngoài

```c
int status = system("ls -l");              // chạy lệnh qua shell (tiện nhưng nguy hiểm nếu ghép chuỗi từ người dùng)
```

**Không** đưa dữ liệu người dùng vào `system()` (nguy cơ **command injection**); nếu cần chạy tiến trình con hãy dùng `fork`/`exec` hoặc `posix_spawn` (POSIX), `CreateProcess` (Windows).

### Thao tác thư mục và metadata (POSIX; Windows có API tương tự)

```c
#include <dirent.h>
#include <sys/stat.h>

DIR *d = opendir(".");
if (d) {
    struct dirent *e;
    while ((e = readdir(d)) != NULL) printf("%s\n", e->d_name);
    closedir(d);
}

struct stat st;
if (stat("data.txt", &st) == 0) {
    printf("kich thuoc: %lld byte\n", (long long)st.st_size);
    printf("la thu muc? %s\n", S_ISDIR(st.st_mode) ? "co" : "khong");
}
```

Các hàm này thuộc **POSIX**, không thuộc chuẩn C. Trên Windows dùng `FindFirstFile`/`FindNextFile`, `GetFileAttributes`, hoặc thư viện đa nền tảng; MinGW cung cấp `dirent.h` và `stat` tương thích.

### Truy cập mức thấp: file descriptor (POSIX)

`open`/`read`/`write`/`close` làm việc với **số nguyên** (file descriptor), **không đệm** ở mức thư viện — nền tảng của socket (chương 16):

```c
#include <fcntl.h>
#include <unistd.h>

int fd = open("data.txt", O_RDONLY);
if (fd >= 0) {
    char buf[256];
    ssize_t n = read(fd, buf, sizeof buf);    // ssize_t có dấu: -1 khi lỗi
    close(fd);
}
```

## 12.10. `mmap` — ánh xạ file vào bộ nhớ (khái niệm)

Thay vì `read`/`fread`, có thể **ánh xạ** file vào không gian địa chỉ và truy cập nó như một mảng byte; hệ điều hành nạp trang từ đĩa khi cần:

```c
#include <sys/mman.h>     // POSIX
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int fd = open("big.dat", O_RDONLY);
struct stat st;
fstat(fd, &st);
const unsigned char *data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
if (data != MAP_FAILED) {
    // dùng data[i] như mảng bình thường
    munmap((void *)data, st.st_size);
}
close(fd);
```

Ưu điểm: nhanh cho truy cập ngẫu nhiên trên file lớn, không sao chép qua bộ đệm. Nhược điểm: **không thuộc chuẩn C**, khó xử lý lỗi I/O (lỗi xuất hiện như tín hiệu `SIGBUS` khi đọc), và **khác biệt giữa nền tảng** (Windows dùng `CreateFileMapping` + `MapViewOfFile`). Khuyến nghị: dùng `fread`/`fwrite` trừ khi cần hiệu năng đặc biệt.

## 12.11. Lỗi thường gặp

| Lỗi | Hậu quả | Cách tránh |
|---|---|---|
| Không kiểm tra `fopen` trả `NULL` | Crash (dùng con trỏ NULL) | `if (!f) { perror(...); }` |
| Quên `fclose` | Mất dữ liệu chưa xả, rò rỉ descriptor | Đóng trên mọi đường thoát |
| `while (!feof(f))` | Xử lý thừa một lần | Kiểm tra giá trị trả về của hàm đọc |
| Dùng `char c = fgetc(f)` | Không phân biệt được EOF | Dùng `int` |
| Mở `"w"` khi định thêm vào file | Mất sạch nội dung cũ | Dùng `"a"` |
| Dùng `"r"`/`"w"` cho dữ liệu nhị phân trên Windows | Dữ liệu bị dịch/cắt | Dùng `"rb"`/`"wb"` |
| Ghi struct thô ra file để chia sẻ | Không di động | Tuần tự hóa từng trường |
| `fgets` rồi quên bỏ `'\n'` | Chuỗi có xuống dòng thừa | `strcspn(line, "\r\n")` |
| Chuyển giữa đọc/ghi không `fseek`/`fflush` | Hành vi không xác định | Gọi `fseek` giữa hai thao tác |
| `system()` với đầu vào người dùng | Command injection | Tránh, hoặc kiểm tra/loại bỏ nghiêm ngặt |

## 12.12. Tóm tắt

- I/O trong C dựa trên **luồng** `FILE *`; `stdin`/`stdout`/`stderr` là ba luồng có sẵn.
- Văn bản: `fgets` + `sscanf`/`strtol` an toàn hơn `fscanf`; nhị phân: `fread`/`fwrite` với `"rb"`/`"wb"`.
- Kiểm tra **mọi** giá trị trả về; dùng `perror`/`strerror(errno)`; `feof` chỉ sau khi đọc thất bại.
- Dữ liệu ghi được **đệm**; `fclose`/`fflush` xả ra; `fsync` để đảm bảo xuống đĩa.
- Ghi struct thô không di động; tuần tự hóa từng trường với kiểu cố định.
- `mmap`, thư mục, `stat` thuộc POSIX chứ không thuộc chuẩn C.

## 12.13. Bài tập

1. Viết chương trình ghi 10 dòng vào file và sau đó đọc lại, in số thứ tự dòng. Thử mở với `"w"` hai lần và quan sát.
2. Viết `mycat` (in nội dung file ra stdout) và `mywc` (đếm dòng/từ/byte) hỗ trợ nhiều file và đọc từ stdin khi không có đối số.
3. Viết chương trình **sao chép file** bằng `fread`/`fwrite`, in ra số byte đã chép; kiểm tra bằng `cmp` (Linux) hoặc `fc /b` (Windows) rằng bản sao giống hệt.
4. Viết chương trình **ghép nhiều file** văn bản vào một file, xử lý file không mở được (báo lỗi, tiếp tục file khác).
5. Viết chương trình đọc file CSV điểm (`ten,diem`), tính điểm trung bình, tìm người điểm cao nhất; bỏ qua và báo cáo dòng lỗi.
6. Viết chương trình lưu/đọc mảng struct `Student` nhị phân bằng cách ghi **từng trường** với kích thước cố định (`uint32_t`, `double` theo định dạng bạn tự quy định), kèm header có magic number và số phiên bản.
7. Viết chương trình tìm chuỗi trong file lớn theo kiểu **streaming**, đúng cả khi mẫu bị cắt ở ranh giới khối (gợi ý: giữ lại `len(mẫu) - 1` byte cuối của khối trước).
8. Viết chương trình đảo ngược thứ tự dòng của file (gợi ý: đọc hết vào danh sách liên kết hoặc dùng `fseek` từ cuối file).
9. (Thử thách) Viết chương trình **ghi nhật ký (logger)** xoay vòng: khi file `app.log` vượt quá 1 MB thì đổi tên thành `app.log.1` và bắt đầu file mới.

Mã nguồn mẫu: /code/chapter-12
