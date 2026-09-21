# Chương 18 — Lời giải bài tập

> Chỉ thử các chương trình lỗi trên máy của bạn, trong môi trường thử nghiệm.

## Bài 1: `vuln.c` với ASan và bảo vệ stack

`gcc -std=c11 -g -fsanitize=address vuln.c -o vuln && ./vuln AAAAAAAAAAAAAAAAAAAAAAAA` cho `stack-buffer-overflow` ở `strcpy`, chỉ ra biến `buf` và độ lệch tràn. Bản vá dùng `snprintf(buf, sizeof buf, "%s", name)` cho báo cáo sạch.

Không dùng ASan, so sánh bảo vệ của compiler: `-fno-stack-protector` → chương trình có thể crash `Segmentation fault` (địa chỉ trở về bị ghi đè); `-fstack-protector-strong` → `*** stack smashing detected ***: terminated` rồi `Aborted` (canary bị phá phát hiện trước khi `ret`).

## Bài 2: format string

```c
// fmt_demo.c
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    printf(argv[1]);                 // SAI: người dùng điều khiển chuỗi định dạng
    printf("\n");
    printf("%s\n", argv[1]);         // ĐÚNG
    return 0;
}
```

`./fmt_demo "%x %x %x %x"` in ra các giá trị đọc từ stack (rò rỉ). Với `gcc -Wall -Wformat-security` bạn nhận `warning: format not a string literal and no format arguments`; thêm `-Werror=format-security` biến nó thành lỗi. Bản đúng là dòng thứ hai (hoặc `puts`).

## Bài 3: cấp phát an toàn tràn số

```c
// xmalloc_array.c
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* NULL nếu n * size tràn hoặc hết bộ nhớ */
static void *xmalloc_array(size_t n, size_t size) {
    if (size != 0 && n > SIZE_MAX / size) return NULL;
    return malloc(n * size);
}

static void *xmalloc_array_builtin(size_t n, size_t size) {       // gcc/clang
    size_t total;
    if (__builtin_mul_overflow(n, size, &total)) return NULL;
    return malloc(total);
}

int main(void) {
    printf("%p\n", xmalloc_array(SIZE_MAX / 2 + 1, 2));           // (nil)
    printf("%p\n", xmalloc_array_builtin(SIZE_MAX / 2 + 1, 2));   // (nil)
    void *p = xmalloc_array(1000, sizeof(int));
    printf("%s\n", p ? "ok" : "loi");
    free(p);
    return 0;
}
```

## Bài 4: `safe_add`, `safe_mul` không UB

```c
// safe_arith.c
#include <limits.h>
#include <stdio.h>

static int safe_add(int a, int b, int *out) {
    if ((b > 0 && a > INT_MAX - b) || (b < 0 && a < INT_MIN - b)) return -1;
    *out = a + b;
    return 0;
}

static int safe_mul(int a, int b, int *out) {
    if (a == 0 || b == 0) { *out = 0; return 0; }
    if ((a == -1 && b == INT_MIN) || (b == -1 && a == INT_MIN)) return -1;
    if (a > 0 ? (b > 0 ? a > INT_MAX / b : b < INT_MIN / a)
              : (b > 0 ? a < INT_MIN / b : b < INT_MAX / a)) return -1;
    *out = a * b;
    return 0;
}

int main(void) {
    int r;
    printf("%d %d\n", safe_add(INT_MAX, 1, &r), safe_add(INT_MAX - 1, 1, &r));       // -1 0
    printf("%d %d\n", safe_add(INT_MIN, -1, &r), safe_mul(INT_MIN, -1, &r));         // -1 -1
    printf("%d %d\n", safe_mul(46341, 46341, &r), safe_mul(46340, 46340, &r));       // -1 0
    return 0;
}
```

Kiểm tra bằng `-fsanitize=undefined`: không có báo cáo tràn số vì mọi phép tính nguy hiểm đã bị chặn **trước** khi thực hiện.

## Bài 5: chống path traversal

```c
// safe_path.c
#define _XOPEN_SOURCE 700              // để có realpath
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Ghép root + name, phân giải và bảo đảm kết quả vẫn nằm TRONG root. Trả 0 nếu an toàn. */
static int resolve_in_root(const char *root, const char *name, char *out, size_t cap) {
    char joined[4096], real_root[4096], real_path[4096];
    if (snprintf(joined, sizeof joined, "%s/%s", root, name) >= (int)sizeof joined) return -1;
    if (!realpath(root, real_root) || !realpath(joined, real_path)) return -1;   // cũng loại file không tồn tại
    size_t n = strlen(real_root);
    if (strncmp(real_path, real_root, n) != 0) return -1;
    if (real_path[n] != '/' && real_path[n] != '\0') return -1;                 // /srv/www-x không khớp /srv/www
    if (strlen(real_path) >= cap) return -1;
    strcpy(out, real_path);
    return 0;
}

int main(int argc, char **argv) {
    char out[4096];
    if (argc < 3) { fprintf(stderr, "cach dung: %s goc ten\n", argv[0]); return 2; }
    if (resolve_in_root(argv[1], argv[2], out, sizeof out) != 0) { puts("TU CHOI"); return 1; }
    printf("OK: %s\n", out);
    return 0;
}
```

Thử `./safe_path ./www index.html` (OK), `../etc/passwd`, `../../etc/passwd`, và một symlink trong `www` trỏ ra ngoài (`ln -s /etc/passwd www/link`) — đều bị `TU CHOI`. (`realpath` cần `_XOPEN_SOURCE`/`_DEFAULT_SOURCE`.)

## Bài 6: fuzz target và vòng lặp fuzz

Với libFuzzer (clang):

```bash
clang -std=c11 -g -O1 -fsanitize=fuzzer,address,undefined fuzz_parse.c parser.c -o fuzz_parse
mkdir -p corpus && ./fuzz_parse corpus -max_total_time=300
```

Khi tìm thấy lỗi, fuzzer ghi `crash-<hash>`; tái hiện bằng `./fuzz_parse crash-<hash>`, đọc báo cáo sanitizer, sửa, rồi **chép file đó vào `tests/regression/`** và thêm một test chạy nó mỗi lần CI. Hạt giống tốt (`corpus/` chứa vài đầu vào hợp lệ) giúp fuzzer nhanh vào sâu logic.

## Bài 7: `consteq` so với `memcmp`

```c
// consteq.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <time.h>

static int consteq(const void *a, const void *b, size_t n) {
    const unsigned char *x = a, *y = b;
    unsigned char diff = 0;
    for (size_t i = 0; i < n; i++) diff |= x[i] ^ y[i];
    return diff == 0;
}

static double bench(int (*cmp)(const void *, const void *, size_t), const char *x, const char *y, size_t n) {
    struct timespec a, b;
    volatile int sink = 0;
    clock_gettime(CLOCK_MONOTONIC, &a);
    for (int i = 0; i < 20000000; i++) sink += cmp(x, y, n);
    clock_gettime(CLOCK_MONOTONIC, &b);
    (void)sink;
    return (double)(b.tv_sec - a.tv_sec) + (double)(b.tv_nsec - a.tv_nsec) * 1e-9;
}

static int memcmp_wrap(const void *a, const void *b, size_t n) { return memcmp(a, b, n) == 0; }

int main(void) {
    char a[64], b_first[64], b_last[64];
    memset(a, 'x', sizeof a); memcpy(b_first, a, sizeof a); memcpy(b_last, a, sizeof a);
    b_first[0] = 'y';                      // khác ở byte ĐẦU
    b_last[63] = 'y';                      // khác ở byte CUỐI
    printf("memcmp  đầu %.3f  cuối %.3f\n", bench(memcmp_wrap, a, b_first, 64), bench(memcmp_wrap, a, b_last, 64));
    printf("consteq đầu %.3f  cuối %.3f\n", bench(consteq, a, b_first, 64), bench(consteq, a, b_last, 64));
    return 0;
}
```

`memcmp` khác biệt rõ giữa hai trường hợp (thoát sớm khi khác ở đầu), `consteq` gần như bằng nhau — đó chính là thuộc tính an toàn cần có. (Cẩn thận: compiler có thể tối ưu vòng lặp; với sản phẩm dùng `CRYPTO_memcmp`/`sodium_memcmp`.)

## Bài 8: xóa bí mật khỏi bộ nhớ

`gcc -O2 -S` cho thấy `memset(password, 0, sizeof password)` ở cuối hàm (khi `password` không được đọc nữa) thường **biến mất** khỏi assembly (dead store elimination), còn `secure_zero` (ghi qua con trỏ `volatile`) hoặc `explicit_bzero` thì được giữ. Tắt echo terminal bằng `termios`: `tcgetattr`, bỏ cờ `ECHO`, `tcsetattr(TCSANOW)`, đọc bằng `fgets`, rồi **khôi phục** thiết lập cũ (kể cả khi lỗi).

## Bài 9: kiểm toán một dự án nhỏ

Mẫu báo cáo: bảng `| Công cụ | Phát hiện | Thật/Dương tính giả | Bản vá |`. Ưu tiên theo mức độ: (1) lỗi bộ nhớ do ASan/fuzzer tái hiện được (chắc chắn thật); (2) `-fanalyzer`/`clang-tidy` báo NULL deref, rò rỉ trên đường lỗi (xác minh bằng cách đọc mã); (3) cảnh báo `cppcheck` về phong cách (thường dương tính giả hoặc ưu tiên thấp). Với mỗi bản vá: thêm một test hồi quy.
