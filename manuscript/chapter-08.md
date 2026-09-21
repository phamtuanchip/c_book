# Chương 8 — Con trỏ chi tiết (Pointers Deep Dive)

## Mục tiêu chương

- Hiểu con trỏ là gì: **một biến chứa địa chỉ bộ nhớ**, và vì sao khái niệm này mạnh mẽ.
- Thành thạo hai toán tử `&` (lấy địa chỉ) và `*` (giải tham chiếu), đọc được khai báo con trỏ.
- Hiểu **số học con trỏ** và mối quan hệ giữa **mảng và con trỏ**.
- Dùng con trỏ để truyền tham chiếu, trả nhiều giá trị, và xây dựng cấu trúc dữ liệu.
- Dùng `const` với con trỏ, `void *`, con trỏ tới con trỏ, con trỏ hàm.
- Nhận diện và tránh các lỗi: con trỏ chưa khởi tạo, `NULL`, con trỏ treo, tràn giới hạn.

> Con trỏ là chủ đề khó nhất của C với người mới, nhưng cũng là thứ làm nên sức mạnh của nó. Hãy đọc chậm, **vẽ sơ đồ bộ nhớ ra giấy** cho mỗi ví dụ và chạy từng ví dụ trên máy của bạn.

## 8.1. Bộ nhớ và địa chỉ

Hình dung bộ nhớ RAM là một dãy dài các **ô nhớ (byte)** được đánh số liên tiếp. Số thứ tự của mỗi ô là **địa chỉ (address)** của nó (thường in ở dạng hex, ví dụ `0x7ffd1234`).

```text
địa chỉ:   0x1000  0x1001  0x1002  0x1003  0x1004  ...
         ┌───────┬───────┬───────┬───────┬───────┐
nội dung │  ...  │  ...  │  ...  │  ...  │  ...  │
         └───────┴───────┴───────┴───────┴───────┘
```

Mỗi biến là một vùng ô nhớ liên tiếp có **địa chỉ** (ô đầu tiên của vùng) và **kích thước** (theo kiểu). Ví dụ `int x = 42;` chiếm 4 byte bắt đầu tại địa chỉ `0x1000`:

```text
địa chỉ    0x1000            (biến x, kiểu int, 4 byte)
         ┌──────────────────┐
         │        42        │
         └──────────────────┘
```

Toán tử **`&`** trả về địa chỉ của một biến:

```c
#include <stdio.h>

int main(void) {
    int x = 42;
    printf("gia tri x     : %d\n", x);
    printf("dia chi cua x : %p\n", (void *)&x);     // %p in địa chỉ; ép sang void* theo chuẩn
    printf("kich thuoc x  : %zu byte\n", sizeof x);
    return 0;
}
```

## 8.2. Con trỏ là gì?

**Con trỏ (pointer)** là một biến mà **giá trị của nó là một địa chỉ** bộ nhớ. Kiểu của con trỏ cho biết dữ liệu **tại địa chỉ đó** thuộc kiểu gì.

```c
int x = 42;
int *p = &x;        // p là con trỏ tới int; p chứa địa chỉ của x
```

Cách đọc khai báo: `int *p` = "`p` là con trỏ tới `int`" (hoặc "`*p` là một `int`").

```text
        biến x                  biến p
   địa chỉ 0x1000          địa chỉ 0x2000
     ┌────────┐              ┌────────────┐
     │   42   │ ◄────────────│   0x1000   │
     └────────┘   p trỏ tới  └────────────┘
```

`p` cũng là một biến, có địa chỉ riêng (`0x2000`) và chiếm bộ nhớ (8 byte trên hệ 64-bit), nhưng giá trị nó lưu là `0x1000` — địa chỉ của `x`.

### Giải tham chiếu (dereference) với `*`

Toán tử `*` đặt **trước một con trỏ** nghĩa là "**truy cập giá trị tại địa chỉ đó**".

```c
int x = 42;
int *p = &x;

printf("%d\n", *p);   // 42: đọc giá trị tại địa chỉ p
*p = 100;             // ghi 100 vào ô nhớ mà p trỏ tới -> x cũng thành 100
printf("%d\n", x);    // 100
```

Điều quan trọng cần nhớ: `*p` **là** `x` (cùng một ô nhớ, chỉ là một tên khác), không phải bản sao.

**Hai ý nghĩa của dấu `*`** (gây nhầm cho người mới):

| Chỗ dùng | Ý nghĩa |
|---|---|
| Trong **khai báo** `int *p;` | "p là con trỏ tới int" |
| Trong **biểu thức** `*p` | "giá trị tại địa chỉ p" (giải tham chiếu) |
| Giữa hai biểu thức `a * b` | phép nhân |

Còn `&` trong biểu thức một ngôi `&x` nghĩa là "địa chỉ của x" (khác `a & b` là AND bit và `a && b` là AND logic).

### Ví dụ tổng hợp

```c
#include <stdio.h>

int main(void) {
    int a = 10, b = 20;
    int *p = &a;

    printf("*p = %d\n", *p);      // 10
    p = &b;                       // đổi p trỏ sang b (không đổi a hay b)
    printf("*p = %d\n", *p);      // 20
    *p = *p + 5;                  // b = b + 5
    printf("a = %d, b = %d\n", a, b);   // a = 10, b = 25
    return 0;
}
```

Phân biệt hai việc: **`p = &b`** đổi *con trỏ* (nó trỏ nơi khác); **`*p = 5`** đổi *dữ liệu* mà nó trỏ tới.

### `NULL` — con trỏ "không trỏ tới đâu"

`NULL` (trong `<stddef.h>`, `<stdio.h>`, `<stdlib.h>`) là giá trị đặc biệt: con trỏ **không trỏ tới đối tượng hợp lệ nào**.

```c
int *p = NULL;
if (p == NULL) { /* chưa có dữ liệu */ }
if (!p)        { /* cách viết tương đương */ }
*p = 1;        // LỖI: giải tham chiếu NULL -> UB, thường là Segmentation fault
```

**Quy tắc vàng:** *luôn khởi tạo con trỏ* (bằng địa chỉ hợp lệ hoặc `NULL`) và *kiểm tra `NULL` trước khi giải tham chiếu* khi con trỏ có thể `NULL` (kết quả `malloc`, `fopen`, `strchr`...).

## 8.3. Con trỏ và hàm: truyền tham chiếu

Chương 6 cho thấy C truyền tham số **theo giá trị**. Truyền con trỏ cho phép hàm sửa dữ liệu của người gọi và trả về nhiều kết quả.

```c
// Hàm nhận con trỏ để "trả về" thương và dư
void divmod(int a, int b, int *quot, int *rem) {
    *quot = a / b;
    *rem  = a % b;
}

int main(void) {
    int q, r;
    divmod(17, 5, &q, &r);
    printf("17 = 5 * %d + %d\n", q, r);   // q=3, r=2
    return 0;
}
```

Lợi ích thứ hai: **tránh sao chép dữ liệu lớn**. Truyền con trỏ tới `struct` lớn tốn 8 byte thay vì hàng trăm byte:

```c
void print_person(const struct Person *p);   // const: hàm hứa không sửa
```

Hãy sơ đồ hóa `divmod(17, 5, &q, &r)`:

```text
main:   q [?]  r [?]           (địa chỉ giả định: q=0x100, r=0x104)
                 ▲     ▲
divmod:  quot ───┘     │        quot = 0x100, rem = 0x104 (hai bản sao của địa chỉ)
         rem ──────────┘        *quot = 3  ghi vào q; *rem = 2 ghi vào r
```

## 8.4. Số học con trỏ

Có thể **cộng/trừ một số nguyên** cho con trỏ, hoặc **trừ hai con trỏ** cùng kiểu. Quy tắc quan trọng: **phép cộng được nhân với kích thước kiểu mà con trỏ trỏ tới**.

```c
int a[5] = {10, 20, 30, 40, 50};
int *p = &a[0];         // hoặc: int *p = a;

p + 1   // địa chỉ của a[1]: cộng 1 × sizeof(int) = 4 byte
p + 2   // địa chỉ của a[2]
*(p+2)  // giá trị a[2] = 30
```

```text
địa chỉ:    1000   1004   1008   1012   1016
          ┌──────┬──────┬──────┬──────┬──────┐
     a:   │  10  │  20  │  30  │  40  │  50  │
          └──────┴──────┴──────┴──────┴──────┘
             ▲      ▲      ▲
             p     p+1    p+2         (mỗi bước +1 = +4 byte vì kiểu int)
```

Với `char *`, `p + 1` tăng 1 byte; với `double *`, `p + 1` tăng 8 byte.

### Các phép được phép

| Phép toán | Ý nghĩa |
|---|---|
| `p + n`, `p - n` | dịch tới/lui `n` phần tử |
| `p++`, `p--`, `p += n` | dịch chính `p` |
| `p2 - p1` | số **phần tử** giữa hai con trỏ (kiểu `ptrdiff_t`), cùng một mảng |
| `p1 < p2`, `==`... | so sánh (chỉ có nghĩa khi cùng một mảng) |
| ~~`p1 + p2`~~, ~~`p * 2`~~ | **không hợp lệ** |

Chỉ được tính con trỏ **trong phạm vi mảng** hoặc ngay **một vị trí sau phần tử cuối** (`a + 5` với mảng 5 phần tử là hợp lệ để so sánh nhưng **không được giải tham chiếu**). Đi xa hơn là UB.

### Duyệt mảng bằng con trỏ

```c
int a[5] = {10, 20, 30, 40, 50};

// Cách 1: chỉ số
for (int i = 0; i < 5; i++) printf("%d ", a[i]);

// Cách 2: con trỏ chạy
for (int *p = a; p < a + 5; p++) printf("%d ", *p);
```

Hai cách cho kết quả như nhau. Cách 1 dễ đọc hơn trong đa số trường hợp; cách 2 hữu ích khi làm việc với chuỗi hoặc cấu trúc kiểu "con trỏ đầu, con trỏ cuối".

## 8.5. Mảng và con trỏ

Trong hầu hết biểu thức, **tên mảng được tự động chuyển thành con trỏ tới phần tử đầu tiên** (*array-to-pointer decay*). Và toán tử `[]` được định nghĩa qua con trỏ:

```text
a[i]   ≡   *(a + i)
```

Do đó `a[2]`, `*(a + 2)` và (đùa vui) `2[a]` đều cho cùng kết quả.

```c
int a[5] = {10, 20, 30, 40, 50};
int *p = a;

printf("%d %d %d\n", a[2], *(a + 2), p[2]);   // 30 30 30
```

Tuy nhiên **mảng KHÔNG phải là con trỏ**. Khác biệt:

| | Mảng `int a[5]` | Con trỏ `int *p` |
|---|---|---|
| `sizeof` | 20 (cả mảng) | 8 (chỉ con trỏ) |
| `&` | `&a` là địa chỉ của cả mảng | `&p` là địa chỉ của biến p |
| Gán | `a = ...` **không được** | `p = ...` được |
| Bộ nhớ | vùng dữ liệu nằm ngay trong biến | chỉ chứa một địa chỉ, dữ liệu ở nơi khác |

Ngoại lệ "phân rã" xảy ra khi tên mảng là toán hạng của `sizeof` hoặc `&`, hoặc là chuỗi literal dùng để khởi tạo mảng.

### Chuỗi và `char *`

```c
const char *s = "hello";
for (const char *p = s; *p != '\0'; p++) {
    putchar(*p);
}

// Đếm độ dài bằng con trỏ
size_t my_strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return (size_t)(p - s);      // hiệu hai con trỏ = số phần tử
}
```

Các dạng viết cô đọng thường thấy trong mã C, ví dụ sao chép chuỗi:

```c
void my_strcpy(char *dst, const char *src) {
    while ((*dst++ = *src++) != '\0') { }
}
```

Giải thích: `*dst++ = *src++` gán ký tự tại `src` cho `*dst`, rồi tăng cả hai con trỏ; giá trị của phép gán là ký tự vừa chép; khi đó là `'\0'`, vòng lặp dừng (và `'\0'` đã được chép). Chú ý (như mọi `strcpy`) nó không kiểm tra kích thước đích.

## 8.6. Con trỏ và `const`

Vị trí của `const` xác định điều gì là "chỉ đọc". Quy tắc đọc **từ phải sang trái**:

```c
int x = 1, y = 2;

const int *p1 = &x;        // p1: con trỏ tới "const int"  -> KHÔNG sửa được *p1, ĐỔI được p1
int * const p2 = &x;       // p2: con trỏ hằng tới int    -> sửa được *p2, KHÔNG đổi được p2
const int * const p3 = &x; // cả hai đều không đổi được
```

```c
*p1 = 5;    // LỖI: dữ liệu là const
p1 = &y;    // OK

*p2 = 5;    // OK
p2 = &y;    // LỖI: con trỏ là const
```

Dùng `const int *` cho tham số của hàm chỉ **đọc**:

```c
size_t count_char(const char *s, char c);   // hàm hứa không sửa chuỗi
```

Nhờ vậy người gọi có thể truyền cả chuỗi literal lẫn mảng, và compiler bắt lỗi nếu hàm lỡ sửa dữ liệu.

## 8.7. `void *` — con trỏ tổng quát

`void *` là con trỏ **không gắn với kiểu nào**. Nó có thể chứa địa chỉ của bất kỳ đối tượng nào, nhưng **không thể giải tham chiếu hay làm số học** trực tiếp — phải ép về kiểu cụ thể trước.

```c
int x = 5;
void *v = &x;               // OK: gán bất kỳ con trỏ nào cho void*
int *ip = v;                // OK trong C: void* tự chuyển về kiểu khác (C++ thì phải ép)
printf("%d\n", *(int *)v);  // ép rồi giải tham chiếu
// printf("%d\n", *v);      // LỖI: không biết đọc bao nhiêu byte
```

`malloc` trả về `void *` nên bạn có thể gán nó thẳng vào bất kỳ con trỏ nào. Ví dụ điển hình khác là hàm hoán đổi tổng quát:

```c
#include <string.h>

void swap_bytes(void *a, void *b, size_t size) {
    unsigned char tmp[64];                 // giả sử size <= 64
    memcpy(tmp, a, size);
    memcpy(a, b, size);
    memcpy(b, tmp, size);
}

int x = 1, y = 2;
swap_bytes(&x, &y, sizeof x);              // dùng cho int
double d1 = 1.5, d2 = 2.5;
swap_bytes(&d1, &d2, sizeof d1);           // dùng cho double
```

Đây chính là cách `qsort` và `bsearch` hoạt động với mọi kiểu dữ liệu: chúng làm việc với `void *` và kích thước phần tử.

## 8.8. Con trỏ tới con trỏ

Con trỏ cũng là biến nên có địa chỉ, và ta có thể có con trỏ trỏ tới nó.

```c
int x = 5;
int *p = &x;
int **pp = &p;            // pp là con trỏ tới (con trỏ tới int)

**pp = 9;                 // *pp là p; **pp là x  -> x = 9
```

```text
   pp ──► p ──► x
 0x3000  0x2000  0x1000
   │        │      └─ 5 (sau đó thành 9)
   │        └─ lưu 0x1000
   └─ lưu 0x2000
```

### Ứng dụng 1: hàm sửa được **chính con trỏ** của người gọi

Khi hàm cần đổi con trỏ (ví dụ cấp phát bộ nhớ và trả qua tham số), phải nhận `T **`:

```c
#include <stdlib.h>

// SAI: chỉ sửa bản sao của con trỏ
void alloc_bad(int *p) {
    p = malloc(sizeof(int));     // p là bản sao; con trỏ ở main không đổi -> rò rỉ bộ nhớ
}

// ĐÚNG: nhận địa chỉ của con trỏ
int alloc_good(int **out) {
    *out = malloc(sizeof(int));  // sửa con trỏ của người gọi
    return *out ? 0 : -1;
}

int main(void) {
    int *q = NULL;
    if (alloc_good(&q) == 0) {
        *q = 7;
        free(q);
    }
    return 0;
}
```

### Ứng dụng 2: mảng các chuỗi (`char **`)

```c
const char *names[] = {"An", "Binh", "Cuong"};   // mảng 3 con trỏ tới chuỗi literal

for (int i = 0; i < 3; i++) printf("%s\n", names[i]);
```

```text
names:  ┌─────┐
        │  ●──┼──► "An"
        ├─────┤
        │  ●──┼──► "Binh"
        ├─────┤
        │  ●──┼──► "Cuong"
        └─────┘
```

Chính là kiểu của `argv` trong `int main(int argc, char *argv[])` — mảng các chuỗi tham số dòng lệnh:

```c
#include <stdio.h>

int main(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);   // argv[0] là tên chương trình
    }
    return 0;
}
```

Chạy `./prog xin chao` in `argv[0] = ./prog`, `argv[1] = xin`, `argv[2] = chao`.

## 8.9. Con trỏ tới struct

Toán tử `->` truy cập thành viên qua con trỏ: `p->x` tương đương `(*p).x`.

```c
struct Point { int x, y; };

struct Point pt = {3, 4};
struct Point *pp = &pt;

printf("%d %d\n", pp->x, pp->y);
pp->x = 10;                       // sửa pt.x
```

Chi tiết `struct` ở chương 10; ở chương 9 và 10 bạn sẽ dùng nhiều con trỏ tới struct để xây dựng danh sách liên kết và cây.

## 8.10. Con trỏ hàm (ôn lại)

Con trỏ có thể trỏ tới **mã** của một hàm (chương 6). Ví dụ bảng phân phối lệnh:

```c
#include <stdio.h>
#include <string.h>

static void cmd_start(void) { puts("start"); }
static void cmd_stop(void)  { puts("stop"); }

typedef void (*Handler)(void);

struct Command { const char *name; Handler fn; };

int main(void) {
    struct Command table[] = { {"start", cmd_start}, {"stop", cmd_stop} };
    size_t n = sizeof table / sizeof table[0];
    const char *input = "stop";
    for (size_t i = 0; i < n; i++) {
        if (strcmp(table[i].name, input) == 0) { table[i].fn(); break; }
    }
    return 0;
}
```

Mẫu "bảng con trỏ hàm" thay cho chuỗi `if/else` dài là cách rất phổ biến trong trình thông dịch, giao thức, và máy trạng thái (chương 19).

## 8.11. Cách đọc khai báo phức tạp

Quy tắc **"xoắn ốc"/"từ trong ra ngoài"**: bắt đầu từ tên biến, đọc sang phải khi gặp `[]` hoặc `()`, sang trái khi gặp `*`, và lặp lại.

| Khai báo | Đọc là |
|---|---|
| `int *p` | p là con trỏ tới int |
| `int a[3]` | a là mảng 3 int |
| `int *a[3]` | a là mảng 3 con trỏ tới int |
| `int (*p)[3]` | p là con trỏ tới mảng 3 int |
| `int *f(void)` | f là hàm trả về con trỏ tới int |
| `int (*f)(void)` | f là con trỏ tới hàm không tham số trả về int |
| `int (*fa[4])(int)` | fa là mảng 4 con trỏ tới hàm nhận int, trả về int |
| `const char *s` | s là con trỏ tới const char |
| `char * const s` | s là hằng con trỏ tới char |

Khi khai báo quá phức tạp, hãy dùng `typedef` để gọi tên từng phần.

## 8.12. Bộ nhớ động (giới thiệu)

Con trỏ là cách duy nhất để dùng bộ nhớ **cấp phát động** trên heap:

```c
#include <stdlib.h>

int *arr = malloc(5 * sizeof *arr);        // xin 5 int
if (arr == NULL) { /* hết bộ nhớ */ }
for (int i = 0; i < 5; i++) arr[i] = i * i;
free(arr);                                  // trả lại
arr = NULL;
```

Mẹo: viết `sizeof *arr` thay vì `sizeof(int)` để khi đổi kiểu của `arr`, kích thước tự cập nhật đúng. Chương 9 nói chi tiết về `malloc`, `realloc`, `free` và các lỗi bộ nhớ.

## 8.13. Các lỗi con trỏ kinh điển

### 1. Con trỏ chưa khởi tạo (wild pointer)

```c
int *p;        // giá trị rác — trỏ tới đâu đó ngẫu nhiên
*p = 42;       // ghi vào địa chỉ ngẫu nhiên -> crash hoặc phá dữ liệu
```

Sửa: luôn khởi tạo (`int *p = NULL;` hoặc trỏ tới biến thật).

### 2. Giải tham chiếu `NULL`

```c
char *s = strchr("abc", 'x');   // không tìm thấy -> NULL
printf("%c\n", *s);             // crash
```

Sửa: kiểm tra `if (s != NULL)`.

### 3. Con trỏ treo (dangling pointer)

```c
int *p = malloc(sizeof *p);
free(p);
*p = 1;                  // use-after-free: vùng nhớ đã trả lại

int *f(void) { int x; return &x; }   // trả về địa chỉ biến cục bộ
```

Sửa: đặt `p = NULL` ngay sau `free`; không trả địa chỉ biến cục bộ.

### 4. Nhầm giữa `p` và `*p`

```c
int x = 5;
int *p = &x;
p = 10;         // LỖI: gán số 10 cho con trỏ (compiler cảnh báo); ý định thường là *p = 10
```

### 5. Sai kiểu / sai kích thước

```c
int *p = malloc(10);             // 10 byte, không phải 10 int
p[9] = 0;                        // ghi ngoài vùng (cần 40 byte)
int *q = malloc(10 * sizeof *q); // đúng
```

### 6. Truy cập ngoài giới hạn mảng

```c
int a[3];
int *p = a + 5;                  // đã UB (đi quá xa)
```

### 7. So sánh con trỏ thay vì nội dung

```c
char *a = "abc", *b = "abc";
if (a == b) { ... }              // so sánh địa chỉ! Dùng strcmp(a, b) == 0
```

### 8. Dùng con trỏ sau `realloc`

```c
int *p = malloc(4 * sizeof *p);
int *q = p;
p = realloc(p, 100 * sizeof *p);   // vùng cũ có thể bị di chuyển
q[0] = 1;                          // q có thể đã "treo"
```

### Công cụ bắt lỗi con trỏ

```bash
gcc -std=c11 -g -fsanitize=address,undefined -o prog prog.c && ./prog
valgrind --leak-check=full ./prog     # Linux
```

Sanitizer chỉ ra chính xác dòng gây lỗi và loại lỗi (heap-use-after-free, stack-buffer-overflow, null dereference...).

## 8.14. Thực hành hoàn chỉnh: đảo ngược mảng và tìm chuỗi con bằng con trỏ

```c
// ptr_practice.c
#include <stdio.h>
#include <string.h>

// Đảo mảng bằng hai con trỏ từ hai đầu
void reverse_ints(int *first, int *last) {     // last trỏ tới phần tử cuối
    while (first < last) {
        int t = *first;
        *first++ = *last;
        *last-- = t;
    }
}

// Tìm chuỗi con bằng con trỏ; trả về con trỏ tới lần xuất hiện đầu hoặc NULL
const char *find_sub(const char *hay, const char *needle) {
    if (*needle == '\0') return hay;
    for (; *hay; hay++) {
        const char *h = hay, *n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (*n == '\0') return hay;            // khớp hết needle
    }
    return NULL;
}

int main(void) {
    int a[] = {1, 2, 3, 4, 5};
    size_t n = sizeof a / sizeof a[0];
    reverse_ints(a, a + n - 1);
    for (size_t i = 0; i < n; i++) printf("%d ", a[i]);     // 5 4 3 2 1
    printf("\n");

    const char *text = "the quick brown fox";
    const char *pos = find_sub(text, "brown");
    if (pos) printf("tim thay tai chi so %td: %s\n", pos - text, pos);   // %td cho ptrdiff_t
    return 0;
}
```

## 8.15. Tóm tắt

- Con trỏ là biến chứa **địa chỉ**; `&x` lấy địa chỉ, `*p` truy cập dữ liệu tại địa chỉ.
- Số học con trỏ nhân theo kích thước kiểu; `a[i] ≡ *(a + i)`.
- Tên mảng phân rã thành con trỏ khi truyền vào hàm, nhưng **mảng không phải con trỏ**.
- `const` đặt trước `*` bảo vệ dữ liệu; đặt sau `*` bảo vệ chính con trỏ.
- `void *` là con trỏ tổng quát; `T **` cho phép hàm đổi con trỏ của người gọi.
- Luôn khởi tạo con trỏ, kiểm tra `NULL`, tránh con trỏ treo, và dùng sanitizer để bắt lỗi.

## 8.16. Bài tập

1. Viết chương trình khai báo `int a = 5; int *p = &a;` và in `a`, `&a`, `p`, `*p`, `&p`. Vẽ sơ đồ bộ nhớ tương ứng.
2. Viết hàm `void swap(int *a, int *b)` rồi hàm `void sort3(int *a, int *b, int *c)` sắp xếp ba số tăng dần.
3. Viết hàm `int *find_max(int *a, size_t n)` trả về **con trỏ** tới phần tử lớn nhất (NULL nếu n = 0).
4. Cài đặt `my_strlen`, `my_strcmp`, `my_strchr`, `my_strrev` chỉ dùng con trỏ (không dùng chỉ số `[]`).
5. Viết hàm `int split(char *line, char *fields[], int max)` tách một dòng theo dấu phẩy, lưu con trỏ tới từng trường vào `fields`.
6. Viết chương trình in các đối số dòng lệnh theo thứ tự ngược, và chương trình cộng các số nguyên trong `argv` (dùng `strtol`).
7. Viết hàm `int alloc_matrix(int rows, int cols, int ***out)` cấp phát ma trận động; và hàm giải phóng tương ứng. (Gợi ý: thử cả hai cách: mảng các con trỏ hàng, và một khối liên tiếp với `rows * cols` phần tử.)
8. Đọc khai báo và viết lại bằng tiếng Việt: `char *(*fp)(const char *, int);`, `int (*(*fn)(void))[3];`. Kiểm tra bằng `cdecl.org`.
9. Tìm lỗi trong mỗi đoạn ở mục 8.13 bằng cách viết chương trình nhỏ tái hiện, chạy với AddressSanitizer, và đọc thông báo.
10. (Thử thách) Viết hàm `void *my_memcpy(void *dst, const void *src, size_t n)` chỉ dùng `unsigned char *`. Sau đó viết `my_memmove` xử lý được trường hợp vùng nhớ chồng lấn (gợi ý: chọn chiều sao chép dựa trên so sánh `dst` và `src`).

Mã nguồn mẫu: /code/chapter-08
