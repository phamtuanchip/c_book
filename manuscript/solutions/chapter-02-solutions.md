# Chương 2 — Lời giải bài tập

> Hãy tự làm trước khi xem lời giải. Biên dịch: `gcc -std=c11 -Wall -Wextra -g -o prog prog.c -lm`.

## Bài 1: đổi sang nhị phân và hex

| Thập phân | Nhị phân | Hex |
|---|---|---|
| 10 | `1010` | `0xA` |
| 100 | `1100100` | `0x64` |
| 255 | `11111111` | `0xFF` |

Cách làm: chia liên tiếp cho 2 lấy phần dư rồi đọc từ dưới lên. Với hex, nhóm nhị phân theo từng 4 bit từ phải sang trái: `1100100` = `0110 0100` = `6` `4`. Chạy `bases.c` với các số này để kiểm tra.

## Bài 2: bù hai 8 bit

| Số | Cách tính | Bù hai 8 bit |
|---|---|---|
| −1 | đảo `00000001` → `11111110`, +1 | `11111111` |
| −10 | đảo `00001010` → `11110101`, +1 | `11110110` |
| −128 | bit dấu 1, các bit còn lại 0 | `10000000` |

Kiểm chứng: `print_binary((uint32_t)(int8_t)-10)` in `...11110110` ở byte thấp nhất.

## Bài 3: đếm số bit 1

```c
// popcount.c
#include <stdint.h>
#include <stdio.h>

// Cách 1: kiểm tra từng bit
static int popcount_loop(uint32_t x) {
    int c = 0;
    while (x) {
        c += (int)(x & 1u);
        x >>= 1;
    }
    return c;
}

// Cách 2 (Kernighan): mỗi lần x & (x - 1) xóa đúng một bit 1 thấp nhất
static int popcount_kernighan(uint32_t x) {
    int c = 0;
    while (x) {
        x &= x - 1;
        c++;
    }
    return c;
}

int main(void) {
    uint32_t tests[] = {0, 1, 13, 255, 0xFFFFFFFFu, 0x80000000u};
    for (size_t i = 0; i < sizeof tests / sizeof tests[0]; i++) {
        printf("%10u: %d %d\n", tests[i], popcount_loop(tests[i]), popcount_kernighan(tests[i]));
    }
    return 0;
}
```

Cách Kernighan chạy đúng số lần bằng số bit 1 (nhanh hơn với số thưa bit). GCC/Clang còn có `__builtin_popcount`.

## Bài 4: phân loại ký tự và đổi hoa → thường

```c
// classify_char.c
#include <ctype.h>
#include <stdio.h>

// Đổi chữ hoa thành chữ thường không dùng tolower: 'A' (65) và 'a' (97) chênh 32
static char to_lower_ascii(char c) {
    if (c >= 'A' && c <= 'Z') return (char)(c + ('a' - 'A'));
    return c;
}

int main(void) {
    int c = getchar();
    if (c == EOF) return 1;
    unsigned char ch = (unsigned char)c;

    if (isupper(ch))      printf("'%c' la chu hoa\n", ch);
    else if (islower(ch)) printf("'%c' la chu thuong\n", ch);
    else if (isdigit(ch)) printf("'%c' la chu so\n", ch);
    else                  printf("ky tu khac (ma %d)\n", ch);

    printf("chu thuong: %c\n", to_lower_ascii((char)ch));
    return 0;
}
```

## Bài 5: bảng kích thước

Trên Linux/macOS 64-bit thường là: `char` 1, `short` 2, `int` 4, `long` **8**, `long long` 8, `float` 4, `double` 8, con trỏ 8. Trên Windows 64-bit `long` chỉ **4** (mô hình LLP64), còn lại giống. Chênh lệch ở `long` là lý do phải dùng `<stdint.h>` khi cần độ rộng cố định.

## Bài 6: byte của một `double`

```c
// double_bytes.c
#include <stdio.h>

int main(void) {
    double d = 1.0;
    unsigned char *p = (unsigned char *)&d;
    for (size_t i = 0; i < sizeof d; i++) printf("%02x ", p[i]);
    printf("\n");
    return 0;
}
```

Trên máy little-endian in `00 00 00 00 00 00 f0 3f`, tức số 64 bit `0x3FF0000000000000`. Theo IEEE 754 (1 bit dấu, 11 bit mũ, 52 bit định trị): dấu = 0; mũ `0x3FF` = 1023 nghĩa là số mũ thực = 1023 − 1023 = 0; định trị toàn 0 (phần ẩn là `1.`) nên giá trị là `1.0 × 2⁰ = 1.0`.

## Bài 7: `0.1f + 0.2f` và epsilon

```c
// float_eps.c
#include <math.h>
#include <stdio.h>

int main(void) {
    float a = 0.1f, b = 0.2f, c = 0.3f;
    printf("a + b = %.9f, c = %.9f\n", a + b, c);
    printf("a + b == c ? %s\n", (a + b == c) ? "dung" : "sai");

    // Tìm epsilon nhỏ nhất (theo lũy thừa của 2) khiến so sánh gần đúng thành công
    float diff = fabsf((a + b) - c);
    printf("|(a+b) - c| = %.10g\n", diff);
    if (diff == 0.0f) {
        printf("hai gia tri bang nhau chinh xac\n");
    } else {
        for (float eps = 1.0f; eps > 0.0f; eps /= 2.0f) {
            if (diff > eps) {
                printf("epsilon nho nhat (luy thua 2) de a+b ~= c: %.10g\n", eps * 2.0f);
                break;
            }
        }
    }
    return 0;
}
```

Với `float` phép cộng có thể tình cờ cho đúng `0.3f` (do làm tròn), trong khi với `double` (`0.1 + 0.2 == 0.3`) thì luôn sai. Bài học không đổi: **so sánh với sai số**, và chọn sai số theo độ lớn của giá trị (sai số tương đối, `fabs(x - y) <= eps * fmax(fabs(x), fabs(y))`) chứ không dùng một hằng cố định cho mọi trường hợp.
