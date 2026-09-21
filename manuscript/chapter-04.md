# Chương 4 — Cú pháp & cấu trúc chương trình C

## Mục tiêu chương

- Nắm vững các thành phần cơ bản của cú pháp: token, từ khóa, định danh, literal, toán tử.
- Khai báo, khởi tạo biến và hằng; hiểu **phạm vi** và **thời gian sống** cơ bản.
- Dùng thành thạo các toán tử: số học, so sánh, logic, bit, gán, điều kiện.
- Hiểu **độ ưu tiên** và **thứ tự kết hợp** của toán tử, cùng quy tắc **chuyển đổi kiểu ngầm định**.
- Nhập dữ liệu an toàn với `scanf`/`fgets`/`strtol`.
- Biết cách tách chương trình thành nhiều file (header/source) và so sánh macro với hàm inline.

## 4.1. Token — "từ vựng" của C

Compiler đọc mã nguồn và đầu tiên tách nó thành các **token** (đơn vị từ vựng nhỏ nhất). Có năm loại:

| Loại token | Ví dụ |
|---|---|
| **Từ khóa (keyword)** | `int`, `return`, `if`, `while`, `struct`... (khoảng 44 từ, không thể dùng làm tên) |
| **Định danh (identifier)** | `count`, `total_price`, `main` |
| **Hằng/Literal** | `42`, `3.14`, `'A'`, `"chuoi"` |
| **Toán tử (operator)** | `+`, `-`, `*`, `==`, `&&`, `->` |
| **Dấu phân cách (punctuator)** | `;`, `,`, `(`, `)`, `{`, `}`, `[`, `]` |

Ví dụ `int total = price * 2;` gồm các token: `int` `total` `=` `price` `*` `2` `;`.

### Quy tắc đặt tên (định danh)

- Gồm chữ cái, chữ số và dấu gạch dưới `_`; **không được bắt đầu bằng chữ số**.
- **Phân biệt hoa/thường:** `Count`, `count` và `COUNT` là ba tên khác nhau.
- Không trùng từ khóa (`int`, `for`...).
- Tránh tên bắt đầu bằng `_` theo sau là chữ hoa hoặc hai dấu `__` — dành riêng cho compiler/thư viện.

Quy ước phổ biến trong C: dùng `snake_case` cho biến và hàm (`total_price`, `read_file`), `UPPER_CASE` cho macro và hằng (`MAX_SIZE`). Hãy chọn tên **có ý nghĩa**: `days_in_month` tốt hơn `d`. Ngoại lệ: biến đếm vòng lặp ngắn `i`, `j`, `k` là chuẩn mực.

### Khoảng trắng và dấu chấm phẩy

C không quan tâm đến khoảng trắng và xuống dòng (ngoại trừ để tách token), nên hai đoạn dưới là như nhau với compiler:

```c
int main(void){int a=1;int b=2;return a+b;}
```

```c
int main(void) {
    int a = 1;
    int b = 2;
    return a + b;
}
```

Nhưng con người đọc đoạn sau dễ hơn nhiều. Hãy thụt lề nhất quán (4 dấu cách hoặc 1 tab) và đặt mỗi câu lệnh một dòng.

Câu lệnh kết thúc bằng `;`. Khối lệnh gom nhiều câu lệnh trong `{ ... }`. Lưu ý dấu `;` **không** đặt sau `}` của thân hàm hay thân `if/for/while`.

## 4.2. Kiểu dữ liệu cơ bản

### Kiểu số nguyên

| Kiểu | Kích thước điển hình | Ghi chú |
|---|---|---|
| `char` | 1 byte | Ký tự hoặc số nguyên nhỏ; có dấu hay không tùy nền tảng |
| `signed char` / `unsigned char` | 1 byte | −128..127 / 0..255 |
| `short` | 2 byte | |
| `int` | 4 byte | Kiểu số nguyên "tự nhiên" của máy |
| `long` | 4 (Windows) hoặc 8 (Linux/macOS 64-bit) | |
| `long long` | 8 byte | C99 |
| `unsigned ...` | như kiểu có dấu | Chỉ không âm, phạm vi gấp đôi |

### Kiểu số thực

`float` (~7 chữ số), `double` (~15 chữ số), `long double`. Mặc định hãy dùng `double`; literal `3.14` có kiểu `double`, còn `3.14f` mới là `float`.

### Kiểu `_Bool` / `bool`

Từ C99, `_Bool` chứa 0 hoặc 1. Header `<stdbool.h>` cho phép viết `bool`, `true`, `false`:

```c
#include <stdbool.h>

bool is_even(int n) {
    return n % 2 == 0;
}
```

Bất kỳ giá trị nào gán vào `bool` đều được quy về 0 hoặc 1: `bool b = 42;` cho `b == 1`.

### Kiểu có kích thước cố định

Header `<stdint.h>` cung cấp `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`. Dùng chúng khi độ rộng quan trọng (định dạng file, giao thức mạng, thao tác bit). `size_t` là kiểu số nguyên không dấu dành cho kích thước và chỉ số; `ptrdiff_t` cho hiệu hai con trỏ.

### Kiểu `void`

`void` nghĩa là "không có giá trị". Dùng cho hàm không trả về gì (`void print_hello(void)`) và cho con trỏ tổng quát `void *` (chương 8).

## 4.3. Biến và hằng

### Khai báo và khởi tạo

```c
int age;               // khai báo, giá trị chưa xác định (rác) nếu là biến cục bộ
int count = 0;         // khai báo và khởi tạo
double x = 1.5, y = 2.5;      // nhiều biến cùng kiểu
char grade = 'A';
```

Khai báo là *nói với compiler biến này tồn tại và có kiểu gì*; khởi tạo là *gán giá trị đầu tiên*. **Luôn khởi tạo biến cục bộ.**

Biến **toàn cục** và **`static`** nếu không khởi tạo sẽ tự động bằng 0. Biến **cục bộ** thì **không**.

### Hằng số

Có ba cách tạo hằng:

```c
#define MAX_USERS 100          // macro: thay chữ trước khi biên dịch, không có kiểu
const int max_users = 100;     // biến chỉ đọc, có kiểu, có phạm vi
enum { MAX_NAME = 32 };        // hằng nguyên, dùng được làm kích thước mảng ở mọi chuẩn
```

Khuyến nghị: dùng `const` (và `enum` cho hằng nguyên) khi có thể vì chúng có kiểu và tuân theo phạm vi. Macro `#define` không có kiểu nên dễ gây lỗi khó thấy.

```c
const int limit = 10;
limit = 20;        // LỖI biên dịch: gán cho biến const
```

### Literal (giá trị viết trực tiếp)

| Ví dụ | Kiểu | Ghi chú |
|---|---|---|
| `42` | `int` | thập phân |
| `42L`, `42UL`, `42LL` | `long`, `unsigned long`, `long long` | hậu tố |
| `0x2A`, `052`, `0b101010` | `int` | hex, bát phân, nhị phân (mở rộng) |
| `3.14` | `double` | |
| `3.14f` | `float` | |
| `1e-3` | `double` | ký hiệu khoa học = 0.001 |
| `'A'` | `int` (có giá trị 65) | ký tự |
| `"chuoi"` | `char[6]` (5 ký tự + `\0`) | chuỗi |

## 4.4. Toán tử

### Toán tử số học

| Toán tử | Ý nghĩa | Ví dụ | Kết quả |
|---|---|---|---|
| `+` `-` `*` | cộng, trừ, nhân | `7 * 3` | `21` |
| `/` | chia | `7 / 2` → `3`; `7.0 / 2` → `3.5` | |
| `%` | chia lấy dư (chỉ số nguyên) | `7 % 3` | `1` |
| `++` `--` | tăng/giảm 1 | `i++` | |

**Chia số nguyên:** kết quả bị cắt phần thập phân (về phía 0): `7 / 2 = 3`, `-7 / 2 = -3`. Dấu của `%` theo số bị chia: `-7 % 3 = -1`. Chia hoặc lấy dư cho 0 là UB.

**Tiền tố và hậu tố:**

```c
int a = 5;
int b = a++;   // hậu tố: b = 5 (giá trị cũ), sau đó a = 6
int c = ++a;   // tiền tố: a = 7 trước, rồi c = 7
```

Đừng dùng `++`/`--` hai lần lên cùng một biến trong một biểu thức (`i = i++ + ++i` là UB).

### Toán tử so sánh

`==` bằng, `!=` khác, `<`, `>`, `<=`, `>=`. Kết quả là `int` có giá trị `1` (đúng) hoặc `0` (sai).

> **Lỗi nổi tiếng:** viết `if (x = 5)` (gán) thay vì `if (x == 5)` (so sánh). Cả hai đều biên dịch được! Câu đầu gán 5 vào `x` rồi luôn cho là "đúng". `-Wall` sẽ cảnh báo. Một số lập trình viên viết `if (5 == x)` để lỗi gõ nhầm thành `5 = x` bị compiler bắt.

### Toán tử logic

| Toán tử | Ý nghĩa | Ghi chú |
|---|---|---|
| `&&` | AND | Cả hai đúng |
| `\|\|` | OR | Ít nhất một đúng |
| `!` | NOT | Đảo giá trị |

Trong C, **0 là sai, mọi giá trị khác 0 là đúng**.

**Đánh giá tắt (short-circuit):** `a && b` — nếu `a` sai thì `b` **không được tính**. `a || b` — nếu `a` đúng thì `b` không được tính. Điều này cho phép viết:

```c
if (p != NULL && p->value > 0) { ... }   // an toàn: p->value chỉ được đọc khi p khác NULL
if (n != 0 && total / n > 10) { ... }    // an toàn: không chia cho 0
```

### Toán tử bit

Làm việc trên từng bit của số nguyên.

| Toán tử | Ý nghĩa | Ví dụ (8 bit) |
|---|---|---|
| `&` | AND bit | `0b1100 & 0b1010 = 0b1000` |
| `\|` | OR bit | `0b1100 \| 0b1010 = 0b1110` |
| `^` | XOR bit | `0b1100 ^ 0b1010 = 0b0110` |
| `~` | NOT bit | `~0b00001111 = 0b11110000` |
| `<<` | dịch trái | `1 << 3 = 8` (nhân 2³) |
| `>>` | dịch phải | `16 >> 2 = 4` (chia 2²) |

Các thao tác thông dụng với **cờ bit (bit flag)**:

```c
unsigned int flags = 0;
#define FLAG_READ   (1u << 0)     // 0001
#define FLAG_WRITE  (1u << 1)     // 0010
#define FLAG_EXEC   (1u << 2)     // 0100

flags |= FLAG_READ | FLAG_WRITE;  // bật bit: flags = 0011
flags &= ~FLAG_WRITE;             // tắt bit: flags = 0001
flags ^= FLAG_EXEC;               // đảo bit: flags = 0101
if (flags & FLAG_READ) { ... }    // kiểm tra bit
```

Lưu ý: `&` khác `&&`. `if (a & b)` là AND bit, `if (a && b)` là AND logic. Nhầm hai toán tử này biên dịch được nhưng cho kết quả sai (ví dụ `1 & 2` là `0`, còn `1 && 2` là `1`).

Dịch bit với số âm hoặc số lượng dịch ≥ số bit của kiểu là UB / implementation-defined, nên hãy dùng kiểu **không dấu** khi dịch bit.

### Toán tử gán

`=` gán; `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<=`, `>>=` là dạng viết tắt: `x += 3` tương đương `x = x + 3`.

Phép gán **là một biểu thức có giá trị**, nên `a = b = 0;` là hợp lệ (gán 0 vào `b` rồi gán kết quả cho `a`).

### Toán tử điều kiện (ba ngôi)

```c
int max = (a > b) ? a : b;        // nếu a > b thì max = a, ngược lại max = b
printf("%s\n", n % 2 == 0 ? "chan" : "le");
```

Chỉ dùng cho biểu thức ngắn; nếu lồng nhiều tầng, hãy dùng `if/else`.

### Toán tử `sizeof`, dấu phẩy, ép kiểu

- `sizeof x` hoặc `sizeof(type)`: kích thước theo byte, tính **khi biên dịch** (trừ VLA).
- `(type)expr`: **ép kiểu (cast)** tường minh, ví dụ `(double)a / b`.
- Toán tử dấu phẩy `a, b`: đánh giá `a` rồi `b`, giá trị là `b`. Hiếm khi cần, thường thấy trong `for (i = 0, j = n - 1; i < j; i++, j--)`.

## 4.5. Độ ưu tiên và thứ tự kết hợp

Khi một biểu thức có nhiều toán tử, **độ ưu tiên (precedence)** quyết định toán tử nào tính trước; với các toán tử cùng mức, **thứ tự kết hợp (associativity)** quyết định trái sang phải hay ngược lại.

Bảng rút gọn (từ ưu tiên cao xuống thấp):

| Mức | Toán tử | Kết hợp |
|---|---|---|
| 1 | `()` `[]` `.` `->` `x++` `x--` | trái → phải |
| 2 | `++x` `--x` `+x` `-x` `!` `~` `(type)` `*x` `&x` `sizeof` | phải → trái |
| 3 | `*` `/` `%` | trái → phải |
| 4 | `+` `-` | trái → phải |
| 5 | `<<` `>>` | trái → phải |
| 6 | `<` `<=` `>` `>=` | trái → phải |
| 7 | `==` `!=` | trái → phải |
| 8 | `&` | trái → phải |
| 9 | `^` | trái → phải |
| 10 | `\|` | trái → phải |
| 11 | `&&` | trái → phải |
| 12 | `\|\|` | trái → phải |
| 13 | `?:` | phải → trái |
| 14 | `=` `+=` `-=` ... | phải → trái |
| 15 | `,` | trái → phải |

Những bẫy phổ biến:

```c
if (x & 1 == 0) { ... }     // SAI: == ưu tiên hơn &, nên thành x & (1 == 0) = x & 0
if ((x & 1) == 0) { ... }   // ĐÚNG: dùng ngoặc

int r = a + b << 2;         // dịch trái ưu tiên thấp hơn +, thành (a + b) << 2
```

**Lời khuyên:** *khi không chắc, hãy thêm dấu ngoặc.* Mã có ngoặc dư vẫn dễ đọc hơn mã phải nhớ bảng ưu tiên.

## 4.6. Chuyển đổi kiểu

### Chuyển đổi ngầm định

Khi một biểu thức trộn nhiều kiểu, C tự động chuyển đổi:

1. **Thăng cấp nguyên (integer promotion):** `char`, `short` được chuyển thành `int` trước khi tính toán.
2. **Chuyển đổi số học thông thường:** hai toán hạng được đưa về kiểu "rộng hơn" theo thứ tự `int` → `unsigned int` → `long` → `unsigned long` → `long long` → ... → `float` → `double` → `long double`.

```c
int    a = 7;
double d = a / 2;         // a / 2 là phép chia nguyên = 3, sau đó chuyển thành 3.0
double e = a / 2.0;       // 2.0 là double -> a chuyển thành 7.0 -> kết quả 3.5
double f = (double)a / 2; // ép a thành double trước -> 3.5
```

### Bẫy `signed` và `unsigned`

Khi trộn số có dấu và không dấu, số có dấu bị chuyển thành **không dấu**, và số âm trở thành số rất lớn:

```c
#include <stdio.h>

int main(void) {
    int a = -1;
    unsigned int b = 1;
    if (a < b) printf("a nho hon b\n");
    else       printf("a KHONG nho hon b\n");   // in dòng này! -1 thành 4294967295
    return 0;
}
```

Một biến thể phổ biến nguy hiểm:

```c
size_t n = 0;
for (size_t i = n - 1; i >= 0; i--) { ... }   // i >= 0 luôn đúng với unsigned: vòng lặp vô hạn
```

Quy tắc: **tránh trộn số có dấu và không dấu**; nếu bắt buộc, ép kiểu tường minh và kiểm tra giá trị trước.

### Chuyển đổi làm mất dữ liệu

```c
int big = 300;
unsigned char c = big;   // 300 % 256 = 44: mất dữ liệu (thường có cảnh báo với -Wconversion)
double d = 3.99;
int i = (int)d;          // i = 3: cắt bỏ phần thập phân, KHÔNG làm tròn
```

Để làm tròn dùng `round()` trong `<math.h>`.

## 4.7. Nhập dữ liệu từ bàn phím

### `scanf` — nhanh nhưng dễ sai

```c
int age;
double height;
char word[32];

scanf("%d", &age);
scanf("%lf", &height);        // double dùng %lf khi ĐỌC (còn khi in dùng %f)
scanf("%31s", word);          // đọc một từ, giới hạn 31 ký tự để chừa chỗ cho '\0'
```

Quy tắc bắt buộc nhớ:

1. Với số (`%d`, `%lf`...) phải truyền **địa chỉ**: `&age`. Với mảng ký tự (`%s`) không cần `&` vì tên mảng đã là địa chỉ.
2. **Luôn kiểm tra giá trị trả về** (số mục đọc thành công):

```c
if (scanf("%d", &age) != 1) {
    fprintf(stderr, "Nhap khong hop le\n");
    return 1;
}
```

3. `%s` phải có **giới hạn độ rộng** (`%31s`); nếu không, nhập dài hơn mảng sẽ ghi tràn bộ nhớ.
4. `%d` bỏ qua khoảng trắng đầu vào và **để lại ký tự xuống dòng** trong bộ đệm. Nếu sau đó gọi `fgets` hoặc `getchar`, nó sẽ đọc ngay ký tự `\n` còn sót đó — nguồn lỗi thường gặp của người mới.

Cách dọn phần còn lại của dòng:

```c
int c;
while ((c = getchar()) != '\n' && c != EOF) { }   // bỏ qua đến hết dòng
```

### Cách an toàn hơn: `fgets` + `strtol`

Đọc cả dòng vào bộ đệm, rồi tự phân tích. Cách này kiểm soát được lỗi tốt hơn:

```c
// read_int.c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

// Đọc một số nguyên từ stdin. Trả về 0 nếu thành công, -1 nếu lỗi.
int read_int(const char *prompt, int *out) {
    char buf[64];
    printf("%s", prompt);
    if (fgets(buf, sizeof buf, stdin) == NULL) return -1;

    char *end;
    errno = 0;
    long v = strtol(buf, &end, 10);          // đổi chuỗi thành số, cơ số 10
    if (end == buf) return -1;               // không có chữ số nào
    if (*end != '\n' && *end != '\0') return -1;   // còn ký tự lạ phía sau
    if (errno == ERANGE || v < INT_MIN || v > INT_MAX) return -1;   // ngoài phạm vi int

    *out = (int)v;
    return 0;
}

int main(void) {
    int n;
    if (read_int("Nhap mot so nguyen: ", &n) != 0) {
        fprintf(stderr, "Du lieu khong hop le\n");
        return 1;
    }
    printf("Ban vua nhap: %d\n", n);
    return 0;
}
```

`strtol(s, &end, base)` đổi chuỗi thành `long`, và đặt `end` tại ký tự đầu tiên không chuyển được — nhờ đó bạn phát hiện được đầu vào sai như `"12abc"`. Sẽ dùng lại `read_int` ở nhiều chương sau.

## 4.8. Hàm — cái nhìn đầu tiên

Hàm gom một dãy lệnh thành một đơn vị có tên, có thể tái sử dụng. (Chương 6 sẽ đi sâu; ở đây chỉ để bạn hiểu cấu trúc chương trình.)

```c
// Định nghĩa (definition): có thân hàm
int add(int a, int b) {
    return a + b;
}
```

Cú pháp: `kiểu_trả_về tên(danh_sách_tham_số) { thân }`.

**Prototype (khai báo hàm)** báo cho compiler biết chữ ký của hàm *trước khi* nó được gọi:

```c
int add(int a, int b);        // prototype, kết thúc bằng ';'

int main(void) {
    printf("%d\n", add(2, 3));   // gọi add — compiler đã biết add nhận gì trả gì
    return 0;
}

int add(int a, int b) {       // định nghĩa đặt ở dưới
    return a + b;
}
```

Nếu không có prototype (hay định nghĩa đứng trước lời gọi), compiler C hiện đại báo lỗi/cảnh báo. Prototype thường đặt trong file header `.h` (xem 4.10).

**Tham số truyền theo giá trị:** hàm nhận **bản sao** của đối số; sửa tham số trong hàm không ảnh hưởng biến gốc (chương 6 và 8 mở rộng).

## 4.9. Tiền xử lý — cơ bản

Bộ tiền xử lý chạy trước compiler và làm việc trên **văn bản**.

```c
#include <stdio.h>       // chèn header hệ thống
#include "mylib.h"       // chèn header của dự án

#define PI 3.14159265    // macro không tham số (hằng)
#define AREA(r) (PI * (r) * (r))     // macro có tham số

#ifdef DEBUG
    printf("debug on\n");            // chỉ được biên dịch nếu DEBUG được định nghĩa
#endif
```

Định nghĩa macro từ dòng lệnh: `gcc -DDEBUG -DMAX=100 main.c`.

### Include guard

Nếu một header bị `#include` hai lần trong cùng một file `.c` (trực tiếp hoặc gián tiếp), các định nghĩa bị lặp và gây lỗi. **Include guard** ngăn điều đó:

```c
// math_utils.h
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

int add(int a, int b);
int square(int x);

#endif /* MATH_UTILS_H */
```

Lần include đầu, `MATH_UTILS_H` chưa được định nghĩa nên nội dung được đưa vào và macro được định nghĩa; các lần sau, `#ifndef` sai nên bị bỏ qua. (Nhiều compiler còn hỗ trợ `#pragma once`, tiện nhưng không thuộc chuẩn.)

## 4.10. Macro so với hàm inline

### Macro và mối nguy hiểm

Macro chỉ **thay chữ**, không có kiểu và không có phạm vi:

```c
#define SQUARE(x) ((x) * (x))

int a = 4;
int r1 = SQUARE(a);         // ((a) * (a)) = 16, đúng
int r2 = SQUARE(a + 1);     // ((a + 1) * (a + 1)) = 25 — đúng nhờ ngoặc quanh x

int i = 3;
int r3 = SQUARE(i++);       // ((i++) * (i++)): i++ bị tính HAI lần -> UB
```

Nếu viết thiếu ngoặc, ví dụ `#define BAD(x) x * x`, thì `BAD(a + 1)` thành `a + 1 * a + 1` — sai hoàn toàn. Quy tắc: **bọc mỗi tham số và cả biểu thức trong dấu ngoặc**, và **không truyền biểu thức có tác dụng phụ** (`i++`, gọi hàm) vào macro.

### Hàm inline

```c
static inline int square_int(int x) {
    return x * x;
}
```

- Có kiểu, kiểm tra tham số, và **tham số chỉ được tính một lần**: `square_int(i++)` an toàn.
- Compiler thường tự chèn thân hàm vào chỗ gọi khi tối ưu hóa (`-O2`), nên nhanh như macro.
- Viết `static inline` trong header là cách dùng phổ biến và an toàn.

**Kết luận:** dùng hàm `static inline` thay macro cho phép tính; chỉ dùng macro cho việc mà hàm không làm được (điều kiện `#ifdef`, chuyển chuỗi bằng `#`, `__FILE__`/`__LINE__`...). Chương 14 đi sâu vào macro.

## 4.11. Tổ chức một dự án nhỏ nhiều file

Khi chương trình lớn dần, hãy tách thành các **module**: mỗi module có file header (`.h` — *giao diện*: prototype, kiểu) và file source (`.c` — *cài đặt*).

```text
project/
├── include/
│   └── math_utils.h
├── src/
│   ├── math_utils.c
│   └── main.c
└── Makefile
```

`include/math_utils.h`:

```c
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

int add(int a, int b);
int square(int x);

#endif
```

`src/math_utils.c`:

```c
#include "math_utils.h"

int add(int a, int b)   { return a + b; }
int square(int x)       { return x * x; }
```

`src/main.c`:

```c
#include <stdio.h>
#include "math_utils.h"

int main(void) {
    printf("2 + 3 = %d\n", add(2, 3));
    printf("4^2  = %d\n", square(4));
    return 0;
}
```

Biên dịch (từng bước để thấy rõ compile và link):

```bash
gcc -std=c11 -Wall -Wextra -Iinclude -c src/math_utils.c -o math_utils.o
gcc -std=c11 -Wall -Wextra -Iinclude -c src/main.c       -o main.o
gcc main.o math_utils.o -o app
./app
```

`-Iinclude` chỉ cho compiler tìm header trong thư mục `include`. Việc này được tự động hóa bằng `Makefile` ở chương 11.

## 4.12. Lỗi thường gặp

| Lỗi | Ví dụ | Cách tránh |
|---|---|---|
| Nhầm `=` và `==` | `if (x = 0)` | `-Wall`, thói quen viết cẩn thận |
| Nhầm `&` và `&&` | `if (a & b)` | Nhớ: `&&` cho điều kiện |
| Quên `&` trong `scanf` | `scanf("%d", n)` | Luôn `&n` cho số |
| Dùng `%f` cho `double` trong `scanf` | `scanf("%f", &d)` | Dùng `%lf` |
| Chia nguyên rồi mong kết quả thực | `double x = 1/2;` → `0` | `1.0/2` hoặc `(double)1/2` |
| Trộn signed/unsigned | `-1 < 1u` sai | Tránh, ép kiểu tường minh |
| Thiếu ngoặc trong macro | `#define SQ(x) x*x` | Bọc `((x)*(x))` |
| Thiếu include guard | lỗi `redefinition` | Thêm `#ifndef ... #endif` |
| Dấu `;` sau `if (...)` | `if (x > 0); {...}` | Nhìn kỹ; `-Wall` cảnh báo |

## 4.13. Tóm tắt

- Chương trình gồm các token: từ khóa, định danh, literal, toán tử, dấu phân cách.
- Luôn khởi tạo biến; ưu tiên `const`/`enum` hơn `#define` cho hằng.
- Nhớ độ ưu tiên toán tử và dùng ngoặc khi không chắc; cẩn thận với chia nguyên và trộn signed/unsigned.
- `&&`/`||` đánh giá tắt; 0 là sai, khác 0 là đúng.
- Nhập dữ liệu: kiểm tra giá trị trả về của `scanf`; ưu tiên `fgets` + `strtol` cho dữ liệu đáng tin.
- Tách module bằng header (giao diện) + source (cài đặt), có include guard; ưu tiên `static inline` hơn macro.

## 4.14. Bài tập

1. Viết chương trình nhập bán kính (số thực), in chu vi và diện tích với 2 chữ số thập phân. Dùng `const double PI`.
2. Viết chương trình đổi nhiệt độ Celsius sang Fahrenheit (`F = C * 9 / 5 + 32`). Thử với `9 / 5` và `9.0 / 5` để thấy khác biệt.
3. Viết chương trình nhập số giây và in ra dạng `giờ:phút:giây` (dùng `/` và `%`).
4. Viết các hàm `set_bit`, `clear_bit`, `toggle_bit`, `test_bit` cho `uint32_t` dùng toán tử bit.
5. Viết macro `SQUARE(x)` và chứng minh lỗi khi dùng `SQUARE(i++)`; sau đó viết `square_int` và so sánh kết quả.
6. Dự đoán kết quả rồi kiểm chứng bằng chương trình: `printf("%d %d %d\n", 7 / 2, -7 / 2, -7 % 3);` và `printf("%d\n", 5 + 3 * 2 - 8 / 4 % 3);`.
7. Tách chương trình máy tính (chương 1) thành `calc.h`, `calc.c`, `main.c` với các hàm `add`, `sub`, `mul`, `div_safe`. Viết `Makefile` đơn giản hoặc lệnh biên dịch từng bước.
8. (Thử thách) Viết hàm `int read_int_range(const char *prompt, int lo, int hi)` hỏi lại cho đến khi người dùng nhập số hợp lệ trong `[lo, hi]`.

Mã nguồn mẫu: /code/chapter-04
