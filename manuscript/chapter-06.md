# Chương 6 — Hàm & phạm vi biến (Functions & Scope)

## Mục tiêu chương

- Hiểu hàm là gì, vì sao chia chương trình thành hàm và cách một lời gọi hàm hoạt động trong bộ nhớ.
- Viết hàm với tham số, giá trị trả về, prototype; hiểu **truyền theo giá trị**.
- Phân biệt **phạm vi (scope)**, **thời gian sống (lifetime)** và **liên kết (linkage)**; dùng đúng `static`, `extern`, biến toàn cục.
- Hiểu **stack frame**, **đệ quy** và giới hạn của nó.
- Nhận biết và tránh các lỗi kinh điển: trả về địa chỉ biến cục bộ, biến toàn cục lạm dụng, đệ quy không có điều kiện dừng.
- Làm quen với con trỏ hàm và hàm nhận số lượng tham số thay đổi (variadic).

## 6.1. Vì sao cần hàm?

Hàm là một đoạn mã được **đặt tên**, có thể gọi nhiều lần với đầu vào khác nhau. Lợi ích:

1. **Tái sử dụng:** viết một lần, dùng nhiều nơi.
2. **Chia để trị:** bài toán lớn chia thành bài toán nhỏ, mỗi hàm làm *một việc*.
3. **Dễ kiểm thử và gỡ lỗi:** kiểm tra riêng từng hàm.
4. **Trừu tượng hóa:** người gọi chỉ cần biết hàm *làm gì*, không cần biết *làm thế nào*.

Nguyên tắc: **một hàm làm một việc**, tên hàm là động từ mô tả việc đó (`calculate_average`, `is_valid_email`, `read_config`). Nếu bạn khó đặt tên hoặc phải dùng chữ "và" (`read_and_parse_and_print`), có lẽ hàm đang làm quá nhiều.

## 6.2. Định nghĩa và gọi hàm

### Cú pháp

```c
kiểu_trả_về tên_hàm(kiểu1 tham_số1, kiểu2 tham_số2) {
    // thân hàm
    return giá_trị;
}
```

```c
// Tính trung bình cộng hai số
double average(double a, double b) {
    return (a + b) / 2.0;
}

int main(void) {
    double m = average(3.0, 4.5);   // gọi hàm: 3.0 và 4.5 là ĐỐI SỐ (argument)
    printf("%.2f\n", m);            // 3.75
    return 0;
}
```

**Thuật ngữ:**

- **Tham số (parameter):** biến khai báo trong định nghĩa hàm (`a`, `b`).
- **Đối số (argument):** giá trị thực tế truyền vào khi gọi (`3.0`, `4.5`).

### Hàm không trả về giá trị

Dùng `void`:

```c
void print_line(int n) {
    for (int i = 0; i < n; i++) putchar('-');
    putchar('\n');
}
```

Có thể dùng `return;` (không kèm giá trị) để thoát sớm khỏi hàm `void`.

### Hàm không có tham số: `(void)`

```c
int get_answer(void) { return 42; }
```

Trong C, `int f()` nghĩa là "hàm nhận **số lượng tham số không xác định**" (kiểu cũ, không kiểm tra), còn `int f(void)` nghĩa là **không có tham số**. Luôn dùng `(void)`.

### Prototype (khai báo hàm)

Compiler đọc từ trên xuống, nên hàm phải được **khai báo trước khi gọi**:

```c
#include <stdio.h>

int square(int x);          // prototype: chỉ có chữ ký, kết thúc bằng ;

int main(void) {
    printf("%d\n", square(5));
    return 0;
}

int square(int x) {         // định nghĩa: có thân hàm
    return x * x;
}
```

Prototype cho phép compiler kiểm tra số lượng và kiểu đối số ở mỗi lời gọi. Tên tham số trong prototype là tùy chọn (`int square(int);` cũng được) nhưng nên viết để có tài liệu. Prototype của hàm dùng chung được đặt trong file `.h`.

## 6.3. Truyền tham số theo giá trị

Trong C, mọi tham số **được truyền theo giá trị (pass by value)**: hàm nhận **bản sao** của đối số. Thay đổi bản sao **không ảnh hưởng** biến gốc.

```c
#include <stdio.h>

void increment(int x) {
    x = x + 1;                     // chỉ sửa bản sao
    printf("trong ham: x = %d\n", x);
}

int main(void) {
    int a = 5;
    increment(a);
    printf("trong main: a = %d\n", a);   // vẫn là 5
    return 0;
}
```

```text
trong ham: x = 6
trong main: a = 5
```

### Muốn hàm sửa biến của người gọi? Truyền địa chỉ

Truyền **con trỏ** (địa chỉ của biến). Hàm vẫn nhận bản sao *của con trỏ*, nhưng bản sao đó trỏ vào cùng biến gốc nên ta sửa được nó:

```c
void increment_ptr(int *p) {
    *p = *p + 1;         // *p là "giá trị tại địa chỉ p"
}

int main(void) {
    int a = 5;
    increment_ptr(&a);   // truyền địa chỉ của a
    printf("%d\n", a);   // 6
    return 0;
}
```

Ví dụ kinh điển: hoán đổi hai số.

```c
void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

int x = 1, y = 2;
swap(&x, &y);            // x = 2, y = 1
```

Bạn đã dùng cơ chế này với `scanf("%d", &n)` — `scanf` cần địa chỉ để ghi giá trị vào `n`. Chương 8 giải thích con trỏ chi tiết.

### Trả về nhiều giá trị

Hàm C chỉ `return` một giá trị. Để "trả về" nhiều giá trị, dùng tham số con trỏ đầu ra hoặc `struct`:

```c
// Cách 1: tham số đầu ra
void min_max(const int a[], int n, int *min, int *max) {
    *min = *max = a[0];
    for (int i = 1; i < n; i++) {
        if (a[i] < *min) *min = a[i];
        if (a[i] > *max) *max = a[i];
    }
}

// Cách 2: trả về struct (chương 10)
typedef struct { int min, max; } MinMax;
MinMax find_min_max(const int a[], int n);
```

### Mảng là tham số: đặc biệt

Khi truyền **mảng** cho hàm, thực chất hàm nhận **con trỏ tới phần tử đầu**, không phải bản sao mảng. Vì vậy hàm **có thể sửa** phần tử mảng gốc, và phải truyền thêm **kích thước**:

```c
void fill_zero(int a[], int n) {     // int a[] thực chất là int *a
    for (int i = 0; i < n; i++) a[i] = 0;
}
```

Bên trong hàm, `sizeof(a)` là kích thước **con trỏ**, không phải mảng (bẫy kinh điển, xem chương 7). Nếu hàm không sửa mảng, khai báo `const`: `int sum(const int a[], int n)`.

## 6.4. Giá trị trả về

- Kiểu của biểu thức sau `return` được chuyển sang kiểu trả về của hàm.
- Hàm khác `void` **phải** trả về giá trị trên mọi đường thực thi. Nếu bạn dùng kết quả của hàm mà nó không `return`, hành vi là không xác định. `-Wall` cảnh báo `control reaches end of non-void function`.
- Giá trị trả về nên **được kiểm tra** nếu nó biểu thị lỗi (`fopen` trả `NULL`, `scanf` trả số mục đọc được).

Quy ước phổ biến để báo lỗi: trả về `0` cho thành công, số âm hoặc `-1` cho lỗi; hoặc trả về con trỏ `NULL` khi lỗi. Chương 13 đi sâu vào xử lý lỗi.

## 6.5. Phạm vi (scope)

**Phạm vi** của một tên (biến, hàm) là vùng mã nguồn nơi tên đó có thể được dùng.

### Bốn loại phạm vi trong C

| Phạm vi | Bắt đầu / kết thúc | Ví dụ |
|---|---|---|
| **Khối (block)** | từ khai báo đến `}` của khối chứa nó | biến trong `{ }`, biến vòng `for` |
| **Hàm (function)** | trong toàn hàm | chỉ áp dụng cho nhãn `goto` |
| **Tệp (file)** | từ khai báo đến cuối file | biến/hàm khai báo ngoài mọi hàm |
| **Prototype hàm** | trong dấu ngoặc của prototype | tên tham số trong prototype |

```c
#include <stdio.h>

int g = 100;                // phạm vi file (biến toàn cục)

int main(void) {
    int x = 1;              // phạm vi khối của main
    {
        int y = 2;          // phạm vi khối trong
        printf("%d %d %d\n", g, x, y);
    }
    // printf("%d", y);     // LỖI: y không tồn tại ngoài khối
    for (int i = 0; i < 3; i++) {
        int t = i * 2;      // t được tạo lại mỗi lần lặp
        printf("%d ", t);
    }
    // i, t không dùng được ở đây
    return 0;
}
```

### Che khuất (shadowing)

Biến trong khối trong có thể **trùng tên** với biến ở khối ngoài và *che* nó:

```c
int x = 10;
{
    int x = 20;              // x này che x bên ngoài
    printf("%d\n", x);       // 20
}
printf("%d\n", x);           // 10
```

Shadowing hợp lệ nhưng dễ gây nhầm; bật `-Wshadow` để được cảnh báo.

## 6.6. Thời gian sống (storage duration) và `static`

**Thời gian sống** là khoảng thời gian **khi chương trình chạy** mà biến tồn tại trong bộ nhớ. Đừng nhầm với phạm vi (phần mã nguồn có thể truy cập).

| Loại | Khi nào tồn tại | Nằm ở đâu | Khởi tạo mặc định |
|---|---|---|---|
| **automatic** (biến cục bộ thường) | từ khi vào khối đến khi ra khỏi khối | stack | **rác** (không xác định) |
| **static** (toàn cục và `static`) | suốt chương trình | vùng data/BSS | **0** |
| **allocated** (malloc) | từ `malloc` đến `free` | heap | rác (`calloc` thì 0) |

### `static` cho biến cục bộ

`static` biến cục bộ **giữ giá trị giữa các lần gọi hàm**; nó được khởi tạo **một lần** duy nhất:

```c
int next_id(void) {
    static int id = 0;     // khởi tạo một lần; tồn tại suốt chương trình
    id++;
    return id;
}

// gọi 3 lần: 1, 2, 3
printf("%d %d %d\n", next_id(), next_id(), next_id());
```

Phạm vi vẫn là bên trong hàm, nhưng thời gian sống là toàn chương trình. Lưu ý `static` cục bộ làm hàm **không tái nhập được (not reentrant)** và không an toàn với đa luồng (chương 15).

### `static` cho biến/hàm ở phạm vi file

Với biến hoặc hàm ở phạm vi file, `static` có nghĩa khác: **giới hạn tên trong file `.c` hiện tại** (*internal linkage*). File khác không thể thấy hay gọi nó — giống như "private" trong các ngôn ngữ khác.

```c
// counter.c
static int counter = 0;                 // chỉ dùng được trong counter.c
static void log_msg(const char *m) {    // hàm nội bộ
    /* ... */
}

int counter_next(void) {                // hàm công khai (external linkage)
    log_msg("tang");
    return ++counter;
}
```

**Quy tắc:** *mọi hàm và biến toàn cục không cần chia sẻ với file khác đều nên là `static`.* Điều này tránh xung đột tên và giúp compiler tối ưu.

### Biến toàn cục và `extern`

Biến toàn cục tồn tại suốt chương trình và mọi hàm đều truy cập được. Để dùng nó từ file khác, khai báo bằng `extern` (báo "biến này được định nghĩa ở nơi khác"):

```c
// config.c
int max_connections = 100;      // ĐỊNH NGHĨA (cấp bộ nhớ) - chỉ ở một file

// config.h
extern int max_connections;     // KHAI BÁO (không cấp bộ nhớ)

// main.c
#include "config.h"
printf("%d\n", max_connections);
```

**Hạn chế biến toàn cục**, vì:
- Mọi hàm có thể sửa nó → khó theo dõi ai làm thay đổi giá trị.
- Làm hàm phụ thuộc trạng thái ẩn → khó kiểm thử.
- Không an toàn với đa luồng.

Thay vào đó, hãy truyền dữ liệu qua tham số, hoặc gói trạng thái vào `struct` và truyền con trỏ tới nó.

### `const` và `register` (ngắn gọn)

- `const`: đối tượng không được sửa qua tên này. `const int MAX = 10;`
- `register`: gợi ý cho compiler cất biến vào thanh ghi. Compiler hiện đại tự quyết định nên từ khóa gần như không còn tác dụng; bạn cũng không thể lấy địa chỉ (`&`) của biến `register`.

## 6.7. Hàm gọi hàm: stack frame

Khi bạn gọi một hàm, CPU/compiler tạo một **stack frame** trên stack chứa: đối số (hoặc chỗ lưu chúng), **địa chỉ trở về** (để biết chạy tiếp ở đâu sau khi hàm xong), và các **biến cục bộ**. Khi hàm `return`, frame bị bỏ.

```c
int square(int x) { return x * x; }

int sum_of_squares(int a, int b) {
    int sa = square(a);
    int sb = square(b);
    return sa + sb;
}

int main(void) {
    int r = sum_of_squares(3, 4);
    printf("%d\n", r);
    return 0;
}
```

Trạng thái stack khi đang chạy `square(3)` bên trong `sum_of_squares(3, 4)` (địa chỉ cao ở trên, stack lớn xuống dưới):

```text
┌────────────────────────────┐  ← cao
│ frame của main             │
│   r (chưa có giá trị)      │
├────────────────────────────┤
│ frame của sum_of_squares   │
│   a = 3, b = 4             │
│   sa (chưa có), sb         │
│   địa chỉ trở về (→ main)  │
├────────────────────────────┤
│ frame của square           │
│   x = 3                    │
│   địa chỉ trở về (→ sum_…) │
└────────────────────────────┘  ← thấp (đỉnh stack hiện tại)
```

Khi `square` trả về, frame của nó biến mất và `sa` nhận giá trị 9.

### Lỗi kinh điển: trả về địa chỉ của biến cục bộ

Biến cục bộ nằm trong stack frame. Khi hàm kết thúc, vùng nhớ đó *được coi là trống* và sẽ bị ghi đè bởi lần gọi hàm tiếp theo. Con trỏ trỏ tới nó thành **con trỏ treo (dangling pointer)**.

```c
int *bad(void) {
    int x = 42;
    return &x;              // LỖI: x sẽ biến mất khi hàm kết thúc
}                           // gcc -Wall: warning: function returns address of local variable

char *bad_string(void) {
    char buf[32] = "xin chao";
    return buf;             // LỖI tương tự: buf là mảng cục bộ
}
```

Cách sửa đúng:

```c
// 1. Người gọi cung cấp bộ nhớ
void good1(char *out, size_t size) {
    snprintf(out, size, "xin chao");
}

// 2. Cấp phát trên heap (người gọi phải free)
char *good2(void) {
    char *s = malloc(32);
    if (s) snprintf(s, 32, "xin chao");
    return s;
}

// 3. Trả về hằng chuỗi (tồn tại suốt chương trình)
const char *good3(void) {
    return "xin chao";
}

// 4. static (tồn tại suốt chương trình, nhưng không an toàn đa luồng)
const char *good4(void) {
    static char buf[32] = "xin chao";
    return buf;
}
```

## 6.8. Đệ quy

**Đệ quy** là hàm tự gọi chính nó. Mỗi hàm đệ quy cần:

1. **Trường hợp cơ sở (base case):** điều kiện dừng, không gọi đệ quy nữa.
2. **Bước đệ quy:** gọi lại chính nó với bài toán **nhỏ hơn**, tiến dần đến trường hợp cơ sở.

### Ví dụ: giai thừa

```c
unsigned long long factorial(unsigned int n) {
    if (n <= 1) return 1;              // base case
    return n * factorial(n - 1);       // bước đệ quy
}
```

Chạy tay `factorial(4)`:

```text
factorial(4) = 4 * factorial(3)
                    = 3 * factorial(2)
                          = 2 * factorial(1)
                                = 1            ← base case
                          = 2 * 1 = 2
                    = 3 * 2 = 6
             = 4 * 6 = 24
```

Mỗi lời gọi tạo một stack frame mới; chúng xếp chồng cho tới base case rồi lần lượt được "tháo" ra. Nếu thiếu base case hoặc bài toán không thu nhỏ, đệ quy vô hạn sẽ làm tràn stack (**stack overflow**, chương trình bị crash với `Segmentation fault`).

### Ví dụ: Fibonacci — ngây thơ và cải tiến

```c
// Ngây thơ: độ phức tạp mũ, fib(40) mất vài giây vì tính lại rất nhiều lần
unsigned long fib_slow(unsigned n) {
    if (n < 2) return n;
    return fib_slow(n - 1) + fib_slow(n - 2);
}

// Lặp: tuyến tính, dùng bộ nhớ O(1)
unsigned long fib_fast(unsigned n) {
    unsigned long a = 0, b = 1;
    for (unsigned i = 0; i < n; i++) {
        unsigned long t = a + b;
        a = b;
        b = t;
    }
    return a;
}
```

Bài học: đệ quy thanh lịch nhưng có thể chậm nếu các bài toán con bị tính lặp lại. Lựa chọn thay thế: vòng lặp, hoặc **ghi nhớ (memoization)**.

### Đệ quy đuôi

Nếu lời gọi đệ quy là **việc cuối cùng** hàm làm (*tail call*), compiler có thể biến nó thành vòng lặp khi tối ưu hóa (`-O2`), tránh tăng stack. Tuy nhiên chuẩn C **không đảm bảo** điều này, nên đừng dựa vào nó cho độ sâu lớn.

```c
unsigned long long fact_acc(unsigned n, unsigned long long acc) {
    if (n <= 1) return acc;
    return fact_acc(n - 1, n * acc);    // lời gọi đuôi
}
```

### Khi nào dùng đệ quy?

- Cấu trúc dữ liệu đệ quy tự nhiên: cây, danh sách liên kết, duyệt thư mục.
- Thuật toán chia để trị: sắp xếp trộn, tìm kiếm nhị phân, Tháp Hà Nội.
- Tránh khi độ sâu có thể rất lớn (hàng chục nghìn+) hoặc khi vòng lặp đơn giản hơn.

### Tháp Hà Nội — ví dụ đệ quy đẹp

```c
#include <stdio.h>

// Chuyển n đĩa từ cọc 'from' sang cọc 'to', dùng cọc 'via' làm trung gian
void hanoi(int n, char from, char to, char via) {
    if (n == 0) return;
    hanoi(n - 1, from, via, to);
    printf("Chuyen dia %d: %c -> %c\n", n, from, to);
    hanoi(n - 1, via, to, from);
}

int main(void) {
    hanoi(3, 'A', 'C', 'B');      // 7 bước = 2^3 - 1
    return 0;
}
```

## 6.9. Con trỏ hàm (giới thiệu)

Tên hàm có thể được dùng như một **giá trị** — địa chỉ của mã hàm. Con trỏ hàm cho phép truyền hành vi như một tham số ("callback").

```c
#include <stdio.h>

int add(int a, int b) { return a + b; }
int mul(int a, int b) { return a * b; }

// tham số op là con trỏ tới hàm nhận hai int và trả về int
int apply(int (*op)(int, int), int x, int y) {
    return op(x, y);
}

int main(void) {
    printf("%d\n", apply(add, 3, 4));   // 7
    printf("%d\n", apply(mul, 3, 4));   // 12
    return 0;
}
```

Cú pháp `int (*op)(int, int)`: `op` là con trỏ tới hàm `(int, int) → int` (ngoặc quanh `*op` là bắt buộc). Dùng `typedef` cho dễ đọc: `typedef int (*BinaryOp)(int, int);`.

Ứng dụng tiêu biểu: hàm `qsort` của thư viện chuẩn nhận một hàm so sánh:

```c
#include <stdlib.h>

int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);          // -1, 0 hoặc 1, không bị tràn như x - y
}

int main(void) {
    int a[] = {5, 2, 9, 1, 7};
    qsort(a, 5, sizeof a[0], cmp_int);
    for (int i = 0; i < 5; i++) printf("%d ", a[i]);   // 1 2 5 7 9
    return 0;
}
```

Chương 8 sẽ giải thích chi tiết `void *` và ép kiểu con trỏ.

## 6.10. Hàm nhận số đối số thay đổi (variadic)

`printf` nhận số lượng đối số bất kỳ. Bạn có thể tự viết hàm như vậy bằng `<stdarg.h>`:

```c
#include <stdarg.h>
#include <stdio.h>

// Tính tổng n số nguyên đi sau tham số n
int sum_ints(int n, ...) {
    va_list args;
    va_start(args, n);              // bắt đầu đọc sau tham số cuối cùng có tên (n)
    int total = 0;
    for (int i = 0; i < n; i++) {
        total += va_arg(args, int); // lấy đối số kế tiếp, PHẢI cho đúng kiểu
    }
    va_end(args);                   // dọn dẹp
    return total;
}

int main(void) {
    printf("%d\n", sum_ints(3, 10, 20, 30));   // 60
    return 0;
}
```

Hạn chế: compiler **không kiểm tra** kiểu và số lượng đối số, nên rất dễ sai. Bạn phải có cách báo hàm biết có bao nhiêu đối số (như tham số `n` hoặc chuỗi định dạng). Sai kiểu ở `va_arg` là UB. Dùng có chừng mực.

## 6.11. Quy tắc thiết kế hàm tốt

1. **Ngắn:** lý tưởng nằm gọn trong một màn hình (~30–50 dòng).
2. **Một nhiệm vụ**, tên rõ ràng.
3. **Ít tham số** (≤ 4). Nếu nhiều hơn, gom vào `struct`.
4. **Không phụ thuộc trạng thái toàn cục** khi có thể; hàm "thuần" (cùng đầu vào → cùng đầu ra, không tác dụng phụ) dễ kiểm thử nhất.
5. **Dùng `const`** cho tham số con trỏ không bị sửa.
6. **Kiểm tra đầu vào** và báo lỗi rõ ràng (trả mã lỗi hoặc `NULL`).
7. **Ghi chú hợp đồng:** hàm làm gì, tham số nào được phép `NULL`, ai giải phóng bộ nhớ.

Ví dụ ghi chú hợp đồng:

```c
/*
 * Tính tổng các phần tử của mảng a có n phần tử.
 * Tiền điều kiện: a khác NULL nếu n > 0.
 * Trả về: tổng; không kiểm tra tràn số.
 */
long sum_array(const int *a, size_t n);
```

## 6.12. Lỗi thường gặp

| Lỗi | Hậu quả | Cách tránh |
|---|---|---|
| Trả về địa chỉ biến cục bộ | Con trỏ treo, dữ liệu rác | Trả về bộ nhớ heap, `static`, hoặc dùng bộ đệm do người gọi cấp |
| Kỳ vọng hàm sửa được biến truyền theo giá trị | Biến không đổi | Truyền con trỏ |
| Quên prototype / `#include` | Cảnh báo `implicit declaration` | Khai báo trước, dùng header |
| Đệ quy thiếu base case | Stack overflow | Xác định base case, kiểm tra tiến tới nó |
| Hàm không `return` trên mọi nhánh | UB nếu dùng giá trị | `-Wall` |
| Lạm dụng biến toàn cục | Khó theo dõi, không an toàn luồng | Truyền tham số / dùng struct |
| Dùng `sizeof(arr)` trên tham số mảng | Ra kích thước con trỏ | Truyền kích thước riêng |
| `int f()` thay vì `int f(void)` | Không kiểm tra đối số | Luôn `(void)` |

## 6.13. Tóm tắt

- Hàm gom mã tái sử dụng; C truyền tham số **theo giá trị**; dùng con trỏ để sửa biến của người gọi.
- Phạm vi (nơi thấy tên) khác thời gian sống (khi nào tồn tại). `static` cục bộ giữ giá trị giữa các lần gọi; `static` ở phạm vi file làm tên riêng tư.
- Mỗi lần gọi tạo stack frame; **không trả về địa chỉ biến cục bộ**.
- Đệ quy cần base case và bước thu nhỏ; cân nhắc độ sâu và hiệu năng.
- Con trỏ hàm cho phép truyền hành vi (callback), ví dụ `qsort`.

## 6.14. Bài tập

1. Viết hàm `int max3(int a, int b, int c)`. Viết thêm phiên bản dùng con trỏ trả về hai giá trị: nhỏ nhất và lớn nhất.
2. Viết hàm `void swap_int(int *a, int *b)` và kiểm tra. Sau đó thử viết `swap` nhận tham số `int a, int b` và giải thích vì sao không hoạt động.
3. Viết hàm `bool is_leap_year(int y)` và `int days_in_month(int m, int y)`.
4. Viết hàm `next_id()` dùng biến `static`. Sau đó chuyển thành biến toàn cục và nêu ưu/nhược điểm.
5. Viết hàm đệ quy `int power(int base, unsigned exp)` và phiên bản "bình phương liên tiếp" (`power(b, e) = power(b, e/2)²`) có độ phức tạp `O(log e)`.
6. Viết hàm đệ quy đảo ngược chuỗi tại chỗ và hàm kiểm tra chuỗi đối xứng (palindrome).
7. Tìm lỗi trong đoạn mã sau và sửa (có ít nhất hai lỗi):

```c
char *make_greeting(char *name) {
    char buf[64];
    sprintf(buf, "Xin chao %s", name);
    return buf;
}
```

8. Dùng `qsort` sắp xếp mảng `int` giảm dần; sau đó sắp xếp mảng chuỗi `const char *names[]` theo thứ tự chữ cái (gợi ý: hàm so sánh gọi `strcmp`).
9. (Thử thách) Viết chương trình máy tính dùng **bảng con trỏ hàm**: một mảng `{ '+', add }, { '-', sub }, ...` và tra cứu theo ký tự toán tử người dùng nhập.

Mã nguồn mẫu: /code/chapter-06
