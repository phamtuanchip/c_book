# Chương 7 — Lời giải bài tập

## Bài 1: thống kê mảng số thực

```c
// array_stats.c
#include <stddef.h>
#include <stdio.h>

static double average(const double *a, size_t n) {
    double s = 0;
    for (size_t i = 0; i < n; i++) s += a[i];
    return n ? s / (double)n : 0.0;
}

static void min_max(const double *a, size_t n, double *min, double *max) {   // tiền điều kiện: n >= 1
    *min = *max = a[0];
    for (size_t i = 1; i < n; i++) {
        if (a[i] < *min) *min = a[i];
        if (a[i] > *max) *max = a[i];
    }
}

static size_t count_above(const double *a, size_t n, double threshold) {
    size_t c = 0;
    for (size_t i = 0; i < n; i++) if (a[i] > threshold) c++;
    return c;
}

int main(void) {
    double a[] = {3.5, 1.0, 4.0, 1.5, 9.0};
    size_t n = sizeof a / sizeof a[0];
    double lo, hi, avg = average(a, n);
    min_max(a, n, &lo, &hi);
    printf("tb = %.2f, min = %.1f, max = %.1f, > tb: %zu\n", avg, lo, hi, count_above(a, n, avg));   // 3.80 1.0 9.0 2
    return 0;
}
```

## Bài 2: xoay mảng sang trái `k` vị trí (đảo ba lần)

```c
// rotate.c
#include <stddef.h>
#include <stdio.h>

static void reverse_range(int *a, size_t lo, size_t hi) {       // đảo a[lo .. hi-1]
    while (lo + 1 < hi) {
        hi--;
        int t = a[lo]; a[lo] = a[hi]; a[hi] = t;
        lo++;
    }
}

// Xoay trái k vị trí: đảo [0,k), đảo [k,n), rồi đảo toàn bộ. O(n), O(1) bộ nhớ.
static void rotate_left(int *a, size_t n, size_t k) {
    if (n == 0) return;
    k %= n;
    reverse_range(a, 0, k);
    reverse_range(a, k, n);
    reverse_range(a, 0, n);
}

int main(void) {
    int a[] = {1, 2, 3, 4, 5, 6};
    rotate_left(a, 6, 2);
    for (int i = 0; i < 6; i++) printf("%d ", a[i]);            // 3 4 5 6 1 2
    printf("\n");
    return 0;
}
```

## Bài 3: hợp nhất hai mảng đã sắp xếp

```c
// merge_sorted.c
#include <stddef.h>
#include <stdio.h>

// out phải có chỗ cho na + nb phần tử
static void merge_sorted(const int *a, size_t na, const int *b, size_t nb, int *out) {
    size_t i = 0, j = 0, k = 0;
    while (i < na && j < nb) out[k++] = (a[i] <= b[j]) ? a[i++] : b[j++];
    while (i < na) out[k++] = a[i++];
    while (j < nb) out[k++] = b[j++];
}

int main(void) {
    int a[] = {1, 4, 9}, b[] = {2, 3, 10, 11}, out[7];
    merge_sorted(a, 3, b, 4, out);
    for (int i = 0; i < 7; i++) printf("%d ", out[i]);           // 1 2 3 4 9 10 11
    printf("\n");
    return 0;
}
```

## Bài 4: hàm chuỗi tự cài đặt

```c
// mystring.c
#include <stddef.h>
#include <stdio.h>

static size_t my_strlen(const char *s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

// Sao chép tối đa cap-1 ký tự và luôn kết thúc bằng '\0'. Trả 0 nếu vừa, -1 nếu bị cắt.
static int my_strcpy(char *dst, size_t cap, const char *src) {
    if (cap == 0) return -1;
    size_t i = 0;
    for (; i + 1 < cap && src[i]; i++) dst[i] = src[i];
    dst[i] = '\0';
    return src[i] == '\0' ? 0 : -1;
}

static int my_strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

// Nối src vào dst (dst có kích thước cap). Trả 0 nếu vừa, -1 nếu bị cắt/không hợp lệ.
static int my_strcat(char *dst, size_t cap, const char *src) {
    size_t len = my_strlen(dst);
    if (len >= cap) return -1;
    return my_strcpy(dst + len, cap - len, src);
}

int main(void) {
    char buf[8] = "ab";
    printf("%zu\n", my_strlen(buf));                    // 2
    printf("%d\n", my_strcat(buf, sizeof buf, "cdef")); // 0  (buf = "abcdef")
    printf("%d\n", my_strcat(buf, sizeof buf, "xyz"));  // -1 (bị cắt: "abcdefx")
    printf("%s %d\n", buf, my_strcmp("abc", "abd"));    // abcdefx -1
    return 0;
}
```

## Bài 5: đảo chuỗi và kiểm tra đối xứng (bỏ khoảng trắng, không phân biệt hoa/thường)

```c
// palindrome.c
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void reverse_string(char *s) {
    size_t n = strlen(s);
    for (size_t i = 0; i < n / 2; i++) {
        char t = s[i]; s[i] = s[n - 1 - i]; s[n - 1 - i] = t;
    }
}

static int is_palindrome(const char *s) {
    size_t i = 0, j = strlen(s);
    while (i < j) {
        if (!isalnum((unsigned char)s[i])) { i++; continue; }
        if (!isalnum((unsigned char)s[j - 1])) { j--; continue; }
        if (tolower((unsigned char)s[i]) != tolower((unsigned char)s[j - 1])) return 0;
        i++; j--;
    }
    return 1;
}

int main(void) {
    char s[] = "hello";
    reverse_string(s);
    printf("%s\n", s);                                            // olleh
    printf("%d %d\n", is_palindrome("Was it a car or a cat I saw?"), is_palindrome("abc"));   // 1 0
    return 0;
}
```

## Bài 6: `trim`

```c
// trim.c
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// Xóa khoảng trắng đầu và cuối, sửa tại chỗ; trả về chính s
static char *trim(char *s) {
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    size_t len = strlen(start);
    while (len > 0 && isspace((unsigned char)start[len - 1])) len--;
    memmove(s, start, len);                 // memmove vì hai vùng có thể chồng lấn
    s[len] = '\0';
    return s;
}

int main(void) {
    char s[] = "   xin chao  \n";
    printf("[%s]\n", trim(s));              // [xin chao]
    return 0;
}
```

## Bài 7: đếm số lần xuất hiện chuỗi con

```c
// count_sub.c
#include <stdio.h>
#include <string.h>

// overlap = 0: không chồng lấn ("aaaa" chứa "aa" 2 lần); overlap = 1: có chồng lấn (3 lần)
static int count_sub(const char *s, const char *sub, int overlap) {
    size_t m = strlen(sub);
    if (m == 0) return 0;
    int c = 0;
    for (const char *p = strstr(s, sub); p; p = strstr(overlap ? p + 1 : p + m, sub)) c++;
    return c;
}

int main(void) {
    printf("%d %d\n", count_sub("aaaa", "aa", 0), count_sub("aaaa", "aa", 1));   // 2 3
    return 0;
}
```

## Bài 8: phân tích dãy số bằng `strtol`

```c
// parse_ints.c
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// Đọc các số nguyên cách nhau bởi khoảng trắng; trả số lượng đọc được, hoặc -1 nếu gặp token sai
static int parse_ints(const char *s, int *out, int cap) {
    int n = 0;
    const char *p = s;
    for (;;) {
        while (isspace((unsigned char)*p)) p++;                // tự bỏ khoảng trắng: không dựa vào cách libc đặt `end`
        if (*p == '\0') return n;                              // hết chuỗi: thành công
        if (!isdigit((unsigned char)*p) && *p != '-' && *p != '+') return -1;   // token không phải số

        char *end;
        errno = 0;
        long v = strtol(p, &end, 10);
        if (end == p) return -1;                               // chỉ có dấu, không có chữ số
        if (errno == ERANGE || v < INT_MIN || v > INT_MAX || n >= cap) return -1;
        out[n++] = (int)v;
        p = end;
    }
}

int main(void) {
    int a[16];
    int n = parse_ints("10 20  30\n", a, 16);
    for (int i = 0; i < n; i++) printf("%d ", a[i]);           // 10 20 30
    printf("\n%d\n", parse_ints("1 2 x", a, 16));              // -1
    return 0;
}
```

## Bài 9: xoay ma trận vuông 90° theo chiều kim đồng hồ

Cách tại chỗ: **chuyển vị** rồi **đảo mỗi hàng**.

```c
// rotate_matrix.c
#include <stdio.h>

#define N 3

static void rotate_cw(int m[N][N]) {
    for (int i = 0; i < N; i++)                  // chuyển vị
        for (int j = i + 1; j < N; j++) {
            int t = m[i][j]; m[i][j] = m[j][i]; m[j][i] = t;
        }
    for (int i = 0; i < N; i++)                  // đảo từng hàng
        for (int j = 0; j < N / 2; j++) {
            int t = m[i][j]; m[i][j] = m[i][N - 1 - j]; m[i][N - 1 - j] = t;
        }
}

int main(void) {
    int m[N][N] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    rotate_cw(m);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) printf("%d ", m[i][j]);
        printf("\n");                            // 7 4 1 / 8 5 2 / 9 6 3
    }
    return 0;
}
```

## Bài 10: tràn bộ đệm dưới ASan

Biên dịch `gcc -std=c11 -g -fsanitize=address -o ovf ovf.c` với:

```c
// ovf.c
#include <string.h>

int main(int argc, char **argv) {
    char buf[8];
    if (argc > 1) strcpy(buf, argv[1]);          // LỖI: không kiểm tra độ dài
    return buf[0];
}
```

Chạy `./ovf AAAAAAAAAAAAAAAA` (16 ký tự). ASan báo:

```text
ERROR: AddressSanitizer: stack-buffer-overflow on address 0x7ffc... at pc ...
WRITE of size 17 at 0x7ffc... thread T0
    #0 ... in strcpy
    #1 ... in main ovf.c:6
Address 0x7ffc... is located in stack of thread T0 at offset 40 in frame
    [32, 40) 'buf' (line 5) <== Memory access at offset 40 overflows this variable
```

Đọc: **loại lỗi** (`stack-buffer-overflow`), **thao tác** (ghi 17 byte = 16 ký tự + `'\0'`), **dòng gây lỗi** (6), và **biến bị tràn** (`buf` chiếm `[32, 40)`, truy cập chạm offset 40). Sửa bằng `snprintf(buf, sizeof buf, "%s", argv[1])`.
