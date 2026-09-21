# Chương 4 — Lời giải bài tập

## Bài 1: chu vi và diện tích hình tròn

```c
// circle.c
#include <stdio.h>

int main(void) {
    const double PI = 3.14159265358979323846;
    double r;
    printf("Ban kinh: ");
    if (scanf("%lf", &r) != 1 || r < 0) {          // %lf khi ĐỌC double
        fprintf(stderr, "Ban kinh khong hop le\n");
        return 1;
    }
    printf("Chu vi    = %.2f\n", 2 * PI * r);
    printf("Dien tich = %.2f\n", PI * r * r);
    return 0;
}
```

## Bài 2: Celsius sang Fahrenheit

```c
// temperature.c
#include <stdio.h>

int main(void) {
    double c;
    if (scanf("%lf", &c) != 1) return 1;
    printf("sai:  %.1f\n", c * (9 / 5) + 32);      // 9 / 5 = 1 (chia nguyên!) nên chỉ cộng 32
    printf("dung: %.1f\n", c * 9.0 / 5 + 32);
    return 0;
}
```

Với `c = 100`: bản sai cho `132.0`, bản đúng cho `212.0`. Hằng `9 / 5` là hai số nguyên nên kết quả `1` trước khi nhân.

## Bài 3: giây → giờ:phút:giây

```c
// hms.c
#include <stdio.h>

int main(void) {
    long total;
    if (scanf("%ld", &total) != 1 || total < 0) return 1;
    long h = total / 3600;
    long m = (total % 3600) / 60;
    long s = total % 60;
    printf("%02ld:%02ld:%02ld\n", h, m, s);        // 3725 -> 01:02:05
    return 0;
}
```

## Bài 4: thao tác bit

```c
// bitops.c
#include <stdint.h>
#include <stdio.h>

static uint32_t set_bit(uint32_t x, unsigned n)    { return x | (UINT32_C(1) << n); }
static uint32_t clear_bit(uint32_t x, unsigned n)  { return x & ~(UINT32_C(1) << n); }
static uint32_t toggle_bit(uint32_t x, unsigned n) { return x ^ (UINT32_C(1) << n); }
static int      test_bit(uint32_t x, unsigned n)   { return (int)((x >> n) & 1u); }

int main(void) {
    uint32_t x = 0;
    x = set_bit(x, 3);               // 0b1000
    x = set_bit(x, 0);               // 0b1001
    x = toggle_bit(x, 1);            // 0b1011
    x = clear_bit(x, 3);             // 0b0011
    printf("x = %u, bit0 = %d, bit3 = %d\n", x, test_bit(x, 0), test_bit(x, 3));   // 3, 1, 0
    return 0;
}
```

Dùng số **không dấu** và hằng `UINT32_C(1)` để tránh UB khi dịch bit vào bit dấu; điều kiện `n < 32` là trách nhiệm của người gọi (có thể thêm `assert`).

## Bài 5: `SQUARE(i++)` và `square_int`

```c
// square_demo.c
#include <stdio.h>

#define SQUARE(x) ((x) * (x))
static inline int square_int(int x) { return x * x; }

int main(void) {
    int i = 3;
    int j = 3;
    int k = square_int(j++);                       // j++ chỉ được tính một lần
    printf("square_int(j++) = %d, j = %d\n", k, j);    // 9, 4
    (void)i;
    // SQUARE(i++) mở rộng thành ((i++) * (i++)): sửa i hai lần giữa hai sequence point -> UB
    // int bad = SQUARE(i++);      // bỏ comment và biên dịch với -Wall để xem cảnh báo, thử -fsanitize=undefined
    return 0;
}
```

Lưu ý: ta tách `j++` ra khỏi lời gọi `printf` vì thứ tự đánh giá các đối số của hàm là **unspecified** (chương 3). Điểm chính: hàm `square_int` chỉ tính `j++` **một lần**, còn macro `SQUARE` tính hai lần.

## Bài 6: dự đoán kết quả

- `7 / 2` = `3`; `-7 / 2` = `-3` (làm tròn về 0); `-7 % 3` = `-1` (dấu theo số bị chia). Dòng in ra: `3 -3 -1`.
- `5 + 3 * 2 - 8 / 4 % 3`: `3 * 2 = 6`; `8 / 4 = 2`, rồi `2 % 3 = 2` (`/` và `%` cùng mức, trái sang phải); biểu thức là `5 + 6 - 2 = 9`.

## Bài 7: tách `calc.h` / `calc.c` / `main.c`

```c
// calc.h
#ifndef CALC_H
#define CALC_H

int calc_add(int a, int b);
int calc_sub(int a, int b);
int calc_mul(int a, int b);
/* Trả 0 và ghi kết quả vào *out; trả -1 nếu b == 0. */
int calc_div_safe(int a, int b, int *out);

#endif
```

```c
// calc.c
#include "calc.h"

int calc_add(int a, int b) { return a + b; }
int calc_sub(int a, int b) { return a - b; }
int calc_mul(int a, int b) { return a * b; }

int calc_div_safe(int a, int b, int *out) {
    if (b == 0) return -1;
    *out = a / b;
    return 0;
}
```

```c
// calc_main.c
#include <stdio.h>
#include "calc.h"

int main(void) {
    int q;
    printf("7 + 2 = %d\n", calc_add(7, 2));
    printf("7 * 2 = %d\n", calc_mul(7, 2));
    if (calc_div_safe(7, 0, &q) != 0) printf("khong chia duoc cho 0\n");
    if (calc_div_safe(7, 2, &q) == 0) printf("7 / 2 = %d\n", q);
    return 0;
}
```

Biên dịch: `gcc -std=c11 -Wall -Wextra -c calc.c && gcc -std=c11 -Wall -Wextra calc_main.c calc.o -o calc_demo`.

## Bài 8: `read_int_range`

```c
// read_range.c
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// Hỏi cho đến khi người dùng nhập số nguyên trong [lo, hi]. Trả 0 nếu thành công, -1 nếu hết đầu vào (EOF).
static int read_int_range(const char *prompt, int lo, int hi, int *out) {
    char buf[64];
    for (;;) {
        printf("%s [%d..%d]: ", prompt, lo, hi);
        fflush(stdout);
        if (!fgets(buf, sizeof buf, stdin)) return -1;           // EOF: dừng, tránh lặp vô hạn

        char *end;
        errno = 0;
        long v = strtol(buf, &end, 10);
        if (end == buf || (*end != '\n' && *end != '\0') || errno == ERANGE || v < lo || v > hi) {
            printf("Gia tri khong hop le, thu lai.\n");
            continue;
        }
        *out = (int)v;
        return 0;
    }
}

int main(void) {
    int age;
    if (read_int_range("Nhap tuoi", 1, 120, &age) != 0) return 1;
    printf("Tuoi: %d\n", age);
    return 0;
}
```

Điểm quan trọng: xử lý `EOF` (nếu không, gõ Ctrl+D sẽ lặp vô hạn) và mọi kiểu đầu vào sai (rỗng, chữ, thừa ký tự, tràn, ngoài khoảng).
