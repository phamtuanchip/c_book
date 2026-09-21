# Chương 8 — Lời giải bài tập

## Bài 1: in `a`, `&a`, `p`, `*p`, `&p`

```c
// ptr_basics.c
#include <stdio.h>

int main(void) {
    int a = 5;
    int *p = &a;
    printf("a  = %d\n", a);                  // 5
    printf("&a = %p\n", (void *)&a);         // địa chỉ của a
    printf("p  = %p\n", (void *)p);          // cùng giá trị với &a
    printf("*p = %d\n", *p);                 // 5
    printf("&p = %p\n", (void *)&p);         // địa chỉ của chính biến p (khác &a)
    return 0;
}
```

Sơ đồ (địa chỉ minh họa):

```text
   a (0x1000)          p (0x2000)
  ┌─────────┐         ┌───────────┐
  │    5    │ ◄────── │  0x1000   │
  └─────────┘         └───────────┘
   &a = 0x1000         &p = 0x2000
```

`p` và `&a` bằng nhau; `&p` là địa chỉ của biến con trỏ.

## Bài 2: `swap` và `sort3`

```c
// sort3.c
#include <stdio.h>

static void swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }

static void sort3(int *a, int *b, int *c) {
    if (*a > *b) swap(a, b);
    if (*b > *c) swap(b, c);
    if (*a > *b) swap(a, b);           // sau hai lượt, *c đã là lớn nhất; sắp lại a, b
}

int main(void) {
    int x = 3, y = 1, z = 2;
    sort3(&x, &y, &z);
    printf("%d %d %d\n", x, y, z);      // 1 2 3
    return 0;
}
```

## Bài 3: `find_max` trả về con trỏ

```c
// find_max.c
#include <stddef.h>
#include <stdio.h>

// Trả về con trỏ tới phần tử lớn nhất; NULL nếu n == 0. Không sao chép, người gọi có thể sửa qua con trỏ.
static int *find_max(int *a, size_t n) {
    if (n == 0) return NULL;
    int *best = a;
    for (int *p = a + 1; p < a + n; p++)
        if (*p > *best) best = p;
    return best;
}

int main(void) {
    int a[] = {3, 9, 2, 9, 5};
    int *m = find_max(a, 5);
    if (m) printf("max = %d tai chi so %td\n", *m, m - a);   // 9 tai chi so 1
    *m = 0;                                                    // sửa thẳng phần tử trong mảng
    return 0;
}
```

## Bài 4: hàm chuỗi chỉ dùng con trỏ

```c
// ptr_strings.c
#include <stddef.h>
#include <stdio.h>

static size_t my_strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

static int my_strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

static char *my_strchr(const char *s, int c) {
    for (; *s; s++) if (*s == (char)c) return (char *)s;
    return c == 0 ? (char *)s : NULL;         // chuẩn C: tìm được cả ký tự '\0' kết thúc
}

static void my_strrev(char *s) {
    char *e = s;
    while (*e) e++;
    while (s < --e) {                          // e trỏ tới ký tự cuối, s tiến, e lùi
        char t = *s; *s++ = *e; *e = t;
    }
}

int main(void) {
    char s[] = "hello";
    printf("%zu %d %s\n", my_strlen(s), my_strcmp("abc", "abd"), my_strchr(s, 'l'));   // 5 -1 llo
    my_strrev(s);
    printf("%s\n", s);                                                                 // olleh
    return 0;
}
```

## Bài 5: `split` theo dấu phẩy

```c
// split_line.c
#include <stdio.h>
#include <string.h>

// Tách line (SẼ BỊ SỬA) theo dấu phẩy; fields[i] trỏ vào các phần của line. Trả số trường.
static int split(char *line, char *fields[], int max) {
    int n = 0;
    char *p = line;
    while (n < max) {
        fields[n++] = p;
        char *comma = strchr(p, ',');
        if (!comma) break;
        *comma = '\0';
        p = comma + 1;
    }
    return n;
}

int main(void) {
    char line[] = "an,binh,,cuong";
    char *f[8];
    int n = split(line, f, 8);
    for (int i = 0; i < n; i++) printf("[%s]", f[i]);          // [an][binh][][cuong]
    printf("\n");
    return 0;
}
```

Khác `strtok`, cách này giữ được **trường rỗng** (`,,`) và không dùng trạng thái tĩnh.

## Bài 6: đối số dòng lệnh

```c
// args.c
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // (a) in ngược
    for (int i = argc - 1; i >= 1; i--) printf("%s ", argv[i]);
    printf("\n");

    // (b) cộng các số
    long sum = 0;
    for (int i = 1; i < argc; i++) {
        char *end;
        errno = 0;
        long v = strtol(argv[i], &end, 10);
        if (end == argv[i] || *end != '\0' || errno == ERANGE) {
            fprintf(stderr, "khong phai so: %s\n", argv[i]);
            return 1;
        }
        sum += v;
    }
    printf("tong = %ld\n", sum);
    return 0;
}
```

`./args 1 2 3` in `3 2 1` rồi `tong = 6`.

## Bài 7: cấp phát ma trận động

```c
// matrix_alloc.c
#include <stdio.h>
#include <stdlib.h>

// Cách 1: mảng con trỏ hàng — dễ dùng m[i][j], nhưng nhiều lần cấp phát
static int **alloc_rows(int rows, int cols) {
    int **m = malloc((size_t)rows * sizeof *m);
    if (!m) return NULL;
    for (int i = 0; i < rows; i++) {
        m[i] = calloc((size_t)cols, sizeof **m);
        if (!m[i]) {
            while (i--) free(m[i]);                    // dọn những hàng đã cấp
            free(m);
            return NULL;
        }
    }
    return m;
}

static void free_rows(int **m, int rows) {
    if (!m) return;
    for (int i = 0; i < rows; i++) free(m[i]);
    free(m);
}

// Cách 2: một khối liên tiếp rows*cols — nhanh hơn (cache), giải phóng một lần; truy cập a[i * cols + j]
static int *alloc_flat(int rows, int cols) {
    return calloc((size_t)rows * (size_t)cols, sizeof(int));
}

int main(void) {
    int **m = alloc_rows(3, 4);
    if (!m) return 1;
    m[1][2] = 7;
    printf("%d\n", m[1][2]);
    free_rows(m, 3);

    int *f = alloc_flat(3, 4);
    if (!f) return 1;
    f[1 * 4 + 2] = 7;
    printf("%d\n", f[1 * 4 + 2]);
    free(f);
    return 0;
}
```

Yêu cầu bài dùng `int ***out` để trả con trỏ qua tham số; bản trên trả bằng `return` cho gọn, và cùng nguyên lý: hàm cấp phát phải **dọn những gì đã cấp** khi thất bại giữa chừng.

## Bài 8: đọc khai báo

- `char *(*fp)(const char *, int);` — `fp` là **con trỏ tới hàm** nhận `(const char *, int)` và trả về `char *`.
- `int (*(*fn)(void))[3];` — `fn` là con trỏ tới hàm không tham số, trả về **con trỏ tới mảng 3 `int`**.

Cách đọc: bắt đầu từ tên, đi ra ngoài; `*` là "con trỏ tới", `(...)` sau tên là "hàm", `[n]` là "mảng n phần tử".

## Bài 9: tái hiện các lỗi con trỏ

Mỗi lỗi ở mục 8.13 tái hiện được bằng vài dòng; đặt `int main(void)` bao quanh và biên dịch với `-fsanitize=address,undefined`. Thông báo tiêu biểu: `SEGV on unknown address 0x000000000000` (NULL dereference), `heap-use-after-free`, `attempting double-free`, `stack-use-after-return` (cần `ASAN_OPTIONS=detect_stack_use_after_return=1` cho con trỏ tới biến cục bộ đã hết hiệu lực), và `heap-buffer-overflow` cho `malloc(10)` rồi ghi `p[9]` kiểu `int`.

## Bài 10: `my_memcpy` và `my_memmove`

```c
// my_mem.c
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static void *my_memcpy(void *dst, const void *src, size_t n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;                 // KHÔNG đúng nếu hai vùng chồng lấn
    return dst;
}

static void *my_memmove(void *dst, const void *src, size_t n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    if (d == s || n == 0) return dst;
    if (d < s) {                             // đích ở trước nguồn: chép xuôi an toàn
        while (n--) *d++ = *s++;
    } else {                                 // đích ở sau nguồn (có thể chồng lấn): chép ngược
        d += n; s += n;
        while (n--) *--d = *--s;
    }
    return dst;
}

int main(void) {
    char buf[] = "abcdef";
    my_memmove(buf + 2, buf, 4);             // chồng lấn: "ababcd"
    printf("%s\n", buf);
    char b2[8];
    my_memcpy(b2, "xyz", 4);
    printf("%s\n", b2);
    return 0;
}
```

Điểm mấu chốt: khi đích nằm **sau** nguồn và hai vùng chồng lấn, chép xuôi sẽ ghi đè dữ liệu nguồn chưa đọc, nên phải chép **ngược**.
