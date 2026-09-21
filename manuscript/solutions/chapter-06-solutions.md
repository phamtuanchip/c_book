# Chương 6 — Lời giải bài tập

## Bài 1: `max3` và min/max qua con trỏ

```c
// max3.c
#include <stdio.h>

static int max3(int a, int b, int c) {
    int m = a;
    if (b > m) m = b;
    if (c > m) m = c;
    return m;
}

static void min_max3(int a, int b, int c, int *min, int *max) {
    *min = *max = a;
    if (b < *min) *min = b;
    if (b > *max) *max = b;
    if (c < *min) *min = c;
    if (c > *max) *max = c;
}

int main(void) {
    int lo, hi;
    min_max3(4, -2, 9, &lo, &hi);
    printf("max3 = %d, min = %d, max = %d\n", max3(4, -2, 9), lo, hi);   // 9, -2, 9
    return 0;
}
```

## Bài 2: `swap` đúng và sai

```c
// swap_demo.c
#include <stdio.h>

static void swap_wrong(int a, int b) {     // hoán đổi BẢN SAO: không ảnh hưởng người gọi
    int t = a; a = b; b = t;
}

static void swap_int(int *a, int *b) {
    int t = *a; *a = *b; *b = t;
}

int main(void) {
    int x = 1, y = 2;
    swap_wrong(x, y);
    printf("sau swap_wrong: %d %d\n", x, y);    // 1 2
    swap_int(&x, &y);
    printf("sau swap_int:   %d %d\n", x, y);    // 2 1
    return 0;
}
```

C truyền tham số **theo giá trị**: `swap_wrong` nhận hai bản sao và đổi chúng, còn `x`, `y` của `main` không đổi.

## Bài 3: năm nhuận và số ngày trong tháng

```c
// calendar.c
#include <stdbool.h>
#include <stdio.h>

static bool is_leap_year(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

static int days_in_month(int m, int y) {
    switch (m) {
        case 4: case 6: case 9: case 11: return 30;
        case 2: return is_leap_year(y) ? 29 : 28;
        case 1: case 3: case 5: case 7: case 8: case 10: case 12: return 31;
        default: return -1;
    }
}

int main(void) {
    printf("2000: %d, 1900: %d, 2024: %d\n", is_leap_year(2000), is_leap_year(1900), is_leap_year(2024));   // 1 0 1
    printf("thang 2/2024 co %d ngay\n", days_in_month(2, 2024));                                            // 29
    return 0;
}
```

## Bài 4: `next_id` bằng `static` so với biến toàn cục

```c
// next_id.c
#include <stdio.h>

static int next_id(void) {
    static int id = 0;            // giữ giá trị giữa các lần gọi, chỉ khởi tạo một lần
    return ++id;
}

int g_id = 0;                     // phiên bản toàn cục
static int next_id_global(void) { return ++g_id; }

int main(void) {
    int a = next_id();
    int b = next_id();
    int c = next_id();
    printf("%d %d %d\n", a, b, c);                 // 1 2 3
    a = next_id_global();
    b = next_id_global();
    printf("%d %d\n", a, b);                       // 1 2
    return 0;
}
```

Lưu ý: ta gọi `next_id()` trong từng câu lệnh riêng vì nếu để ba lời gọi trong cùng một `printf`, thứ tự đánh giá đối số là **không xác định** (kết quả có thể là `1 2 3` hoặc `3 2 1`).

- **`static` cục bộ:** dữ liệu chỉ nhìn thấy trong hàm → không ai vô tình sửa từ bên ngoài. Vẫn không an toàn với đa luồng và làm hàm khó kiểm thử (có trạng thái ẩn).
- **Toàn cục:** mọi hàm đều đọc/sửa được (dễ sai, khó theo dõi); cho phép reset trong kiểm thử. Ưu tiên cách thứ nhất, hoặc truyền một `struct` chứa bộ đếm.

## Bài 5: lũy thừa đệ quy

```c
// power.c
#include <stdio.h>

// O(e): nhân e lần
static long power_slow(long base, unsigned exp) {
    if (exp == 0) return 1;
    return base * power_slow(base, exp - 1);
}

// O(log e): power(b, e) = power(b, e/2)^2 (nhân thêm b nếu e lẻ)
static long power_fast(long base, unsigned exp) {
    if (exp == 0) return 1;
    long half = power_fast(base, exp / 2);
    return (exp % 2 == 0) ? half * half : half * half * base;
}

int main(void) {
    printf("%ld %ld\n", power_slow(3, 13), power_fast(3, 13));    // 1594323 1594323
    return 0;
}
```

Chạy tay `power_fast(3, 13)`: 13 → 6 → 3 → 1 → 0, chỉ 5 lời gọi thay vì 14. (Cả hai bản chưa kiểm tra tràn; với `long` 32 bit sẽ tràn sớm.)

## Bài 6: đảo chuỗi và kiểm tra đối xứng đệ quy

```c
// recursion_strings.c
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void reverse_rec(char *s, size_t lo, size_t hi) {    // đảo đoạn s[lo..hi]
    if (lo >= hi) return;
    char t = s[lo]; s[lo] = s[hi]; s[hi] = t;
    reverse_rec(s, lo + 1, hi - 1);
}

// Bỏ qua ký tự không phải chữ/số và phân biệt hoa/thường không quan trọng
static int is_palindrome(const char *s, size_t lo, size_t hi) {
    while (lo < hi && !isalnum((unsigned char)s[lo])) lo++;
    while (lo < hi && !isalnum((unsigned char)s[hi])) hi--;
    if (lo >= hi) return 1;
    if (tolower((unsigned char)s[lo]) != tolower((unsigned char)s[hi])) return 0;
    return is_palindrome(s, lo + 1, hi - 1);
}

int main(void) {
    char s[] = "abcdef";
    reverse_rec(s, 0, strlen(s) - 1);
    printf("%s\n", s);                                                        // fedcba

    const char *p = "A man, a plan, a canal: Panama";
    printf("%d\n", is_palindrome(p, 0, strlen(p) - 1));                       // 1
    return 0;
}
```

Cẩn thận `hi - 1` với `size_t` khi `hi = 0` (quay vòng); điều kiện `lo >= hi` ở đầu hàm chặn trước trường hợp đó.

## Bài 7: tìm lỗi `make_greeting`

```c
char *make_greeting(char *name) {
    char buf[64];
    sprintf(buf, "Xin chao %s", name);
    return buf;
}
```

Hai lỗi: (1) **trả về địa chỉ biến cục bộ** `buf` (biến mất khi hàm kết thúc); (2) `sprintf` **không giới hạn kích thước** — `name` dài hơn ~52 ký tự sẽ tràn `buf`. Bản sửa:

```c
// greeting.c
#include <stdio.h>
#include <stdlib.h>

// Trả về chuỗi cấp phát bằng malloc; NGƯỜI GỌI phải free. NULL nếu hết bộ nhớ hoặc name == NULL.
static char *make_greeting(const char *name) {
    if (!name) return NULL;
    int n = snprintf(NULL, 0, "Xin chao %s", name);        // đo độ dài cần thiết
    if (n < 0) return NULL;
    char *out = malloc((size_t)n + 1);
    if (!out) return NULL;
    snprintf(out, (size_t)n + 1, "Xin chao %s", name);
    return out;
}

int main(void) {
    char *g = make_greeting("An");
    if (g) { puts(g); free(g); }
    return 0;
}
```

## Bài 8: `qsort` giảm dần và sắp xếp chuỗi

```c
// qsort_demo.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cmp_desc(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x < y) - (x > y);                                  // đảo dấu -> giảm dần
}

static int cmp_str(const void *a, const void *b) {
    const char *x = *(const char *const *)a;                   // phần tử là "con trỏ tới chuỗi"
    const char *y = *(const char *const *)b;
    return strcmp(x, y);
}

int main(void) {
    int a[] = {5, 2, 9, 1, 7};
    qsort(a, 5, sizeof a[0], cmp_desc);
    for (int i = 0; i < 5; i++) printf("%d ", a[i]);           // 9 7 5 2 1
    printf("\n");

    const char *names[] = {"Cuong", "An", "Binh"};
    qsort(names, 3, sizeof names[0], cmp_str);
    for (int i = 0; i < 3; i++) printf("%s ", names[i]);       // An Binh Cuong
    printf("\n");
    return 0;
}
```

Điểm dễ sai: với mảng chuỗi, hàm so sánh nhận **con trỏ tới phần tử** (tức `char **`), nên phải ép `const char *const *` rồi giải tham chiếu một lần.

## Bài 9: máy tính dùng bảng con trỏ hàm

```c
// calc_table.c
#include <stdio.h>

typedef double (*BinOp)(double, double);

static double op_add(double a, double b) { return a + b; }
static double op_sub(double a, double b) { return a - b; }
static double op_mul(double a, double b) { return a * b; }
static double op_div(double a, double b) { return b != 0 ? a / b : 0; }

static const struct { char sym; BinOp fn; } TABLE[] = {
    { '+', op_add }, { '-', op_sub }, { '*', op_mul }, { '/', op_div },
};

int main(void) {
    double a, b;
    char op;
    if (scanf("%lf %c %lf", &a, &op, &b) != 3) { fprintf(stderr, "cu phap: so toan_tu so\n"); return 1; }

    for (size_t i = 0; i < sizeof TABLE / sizeof TABLE[0]; i++) {
        if (TABLE[i].sym == op) {
            if (op == '/' && b == 0) { fprintf(stderr, "chia cho 0\n"); return 1; }
            printf("= %g\n", TABLE[i].fn(a, b));
            return 0;
        }
    }
    fprintf(stderr, "toan tu khong ho tro: %c\n", op);
    return 1;
}
```

Thêm toán tử mới chỉ cần viết một hàm và thêm một dòng vào `TABLE`, không sửa logic tra cứu.
