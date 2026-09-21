# Chương 14 — Tiền xử lý & macro nâng cao

## Mục tiêu chương

- Hiểu **bộ tiền xử lý** làm gì, ở giai đoạn nào, và nó chỉ thao tác trên **văn bản**.
- Viết macro có tham số đúng cách, tránh các bẫy: thiếu ngoặc, đánh giá nhiều lần, `;` thừa, tên trùng.
- Dùng `#` (chuỗi hóa) và `##` (nối token); macro biến thiên `__VA_ARGS__`.
- Dùng **X-macro** để sinh mã lặp lại từ một danh sách duy nhất.
- Biên dịch có điều kiện (`#if`, `#ifdef`), macro định sẵn, kiểm tra tính năng/nền tảng.
- Biết khi nào nên dùng hàm `static inline`, `_Generic`, `enum` thay cho macro.

## 14.1. Bộ tiền xử lý làm gì?

Trước khi compiler dịch mã, **bộ tiền xử lý (preprocessor)** biến đổi mã nguồn dạng văn bản:

1. Ghép các dòng kết thúc bằng `\` (nối dòng).
2. Xóa comment (thay bằng một khoảng trắng).
3. Thực hiện các **chỉ thị** bắt đầu bằng `#`: `#include`, `#define`, `#if...`.
4. Thay thế macro.

Nó **không hiểu C**: không biết kiểu, không biết phạm vi, chỉ biết "token" và "chuỗi". Đó vừa là sức mạnh (sinh mã linh hoạt) vừa là nguồn của nhiều lỗi khó thấy.

Xem kết quả tiền xử lý:

```bash
gcc -E main.c            # in mã sau tiền xử lý ra màn hình
gcc -E -P main.c > out.i # bỏ các dòng #line, ghi ra file
```

`gcc -E` là **công cụ gỡ lỗi macro số một**: khi macro cho kết quả kỳ lạ, hãy nhìn xem nó mở rộng thành gì.

### Các chỉ thị chính

| Chỉ thị | Tác dụng |
|---|---|
| `#include` | chèn nội dung file |
| `#define` / `#undef` | định nghĩa / hủy macro |
| `#if` / `#elif` / `#else` / `#endif` | biên dịch có điều kiện theo biểu thức hằng |
| `#ifdef` / `#ifndef` | kiểm tra macro có được định nghĩa không |
| `#error "msg"` | dừng biên dịch với thông báo |
| `#warning "msg"` | cảnh báo (gcc/clang; chuẩn hóa ở C23) |
| `#pragma ...` | chỉ thị đặc thù compiler |
| `#line`, `#` (rỗng) | hiếm dùng |

## 14.2. Macro dạng đối tượng và dạng hàm

### Macro dạng đối tượng (object-like)

```c
#define MAX_USERS 100
#define VERSION   "1.2.0"
#define PI        3.14159265358979323846

char names[MAX_USERS][32];
```

Bộ tiền xử lý thay thế **từng token** `MAX_USERS` bằng `100`. Không có kiểu, không có `;`. Đừng viết `#define MAX_USERS = 100;` (dấu `=` và `;` sẽ đi theo vào mã!).

Với hằng số, ưu tiên `enum { MAX_USERS = 100 };` hoặc `static const`: có kiểu, xuất hiện trong debugger, theo phạm vi.

### Macro dạng hàm (function-like)

```c
#define SQUARE(x) ((x) * (x))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
```

**Không được có khoảng trắng giữa tên macro và dấu `(`** trong định nghĩa: `#define SQUARE (x) ...` là macro dạng đối tượng mở rộng thành `(x) ...`.

## 14.3. Các bẫy kinh điển của macro

### Bẫy 1: thiếu ngoặc

```c
#define DOUBLE(x) x * 2
int r = DOUBLE(3 + 4);        // mở rộng: 3 + 4 * 2 = 11  (mong đợi 14)
```

```c
#define SQ(x) ((x) * (x))     // ngoặc quanh từng tham số VÀ cả biểu thức
```

Ngoặc quanh **toàn bộ** biểu thức bảo vệ khi macro nằm trong biểu thức lớn hơn:

```c
#define ADD(a, b) (a) + (b)
int r = ADD(1, 2) * 3;        // 1 + 2 * 3 = 7 (mong đợi 9)   -> phải là ((a) + (b))
```

### Bẫy 2: đánh giá tham số nhiều lần

```c
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int i = 5, j = 3;
int m = MAX(i++, j);          // ((i++) > (j) ? (i++) : (j)) -> i tăng HAI lần nếu i > j
```

Nếu đối số có **tác dụng phụ** (`i++`, hàm có side effect, `rand()`), nó bị chạy nhiều lần. Cách khắc phục:

- Dùng hàm `static inline` (mỗi đối số chỉ tính một lần).
- Với gcc/clang: dùng *statement expression* và `typeof` (không chuẩn):

```c
#define MAX_SAFE(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); _a > _b ? _a : _b; })
```

- Hoặc dùng `_Generic` (C11), xem 14.7.

### Bẫy 3: macro nhiều câu lệnh và dấu `;`

```c
#define SWAP(a, b) int t = a; a = b; b = t;

if (x > y)
    SWAP(x, y);               // mở rộng: chỉ "int t = x;" thuộc if; hai câu sau LUÔN chạy!
else ...                      // và "else" bị lỗi cú pháp
```

Cách đúng: bọc trong `do { ... } while (0)` để macro hành xử như **một câu lệnh** và yêu cầu `;` ở cuối:

```c
#define SWAP(type, a, b) do { type _t = (a); (a) = (b); (b) = _t; } while (0)

if (x > y)
    SWAP(int, x, y);          // đúng
else
    ...
```

Mẫu `do { ... } while (0)` là chuẩn mực cho macro nhiều lệnh.

### Bẫy 4: va chạm tên (không có phạm vi)

```c
#define SWAP(a, b) do { int t = (a); (a) = (b); (b) = t; } while (0)

int t = 1, u = 2;
SWAP(t, u);                   // int t = (t); ... -> biến t trong macro che t bên ngoài -> sai
```

Đặt tên biến nội bộ khó trùng (`_t`, `swap_tmp__`) hoặc tránh macro và dùng hàm.

### Bẫy 5: macro không có kiểu và bị nhân đôi mã

Vì là văn bản, `MAX("a", 5)` không gặp lỗi kiểu ở macro mà ở vị trí sử dụng, với thông báo lỗi khó đọc. Và mỗi lần dùng sinh ra một bản mã mới (phình mã).

### Bẫy 6: đệ quy macro và tên trùng hàm

Macro **không đệ quy**: tên macro không được mở rộng lại bên trong chính nó. Và nếu một macro trùng tên với hàm/biến (ví dụ `#define min(a,b)`), nó có thể phá vỡ mã trong header hệ thống. Đặt tên macro bằng **CHỮ HOA** (quy ước) để dễ nhận ra.

## 14.4. Chuỗi hóa `#` và nối token `##`

### `#` — biến tham số thành chuỗi

```c
#define STR(x) #x
#define SHOW(expr) printf(#expr " = %d\n", (expr))

SHOW(3 + 4);                 // printf("3 + 4" " = %d\n", (3 + 4));  -> in: 3 + 4 = 7
puts(STR(hello world));      // "hello world"
```

Chuỗi liền kề tự nối (`"3 + 4" " = %d\n"` thành một chuỗi). Rất hữu ích cho `assert` tự viết, in tên biến, in tên hàm.

**Chuỗi hóa hai bước** để mở rộng macro trước khi chuỗi hóa:

```c
#define VERSION_MAJOR 2
#define STR_(x) #x
#define STR(x) STR_(x)              // qua một tầng để x được mở rộng trước

const char *v = "v" STR(VERSION_MAJOR);   // "v2" (nếu chỉ dùng #x trực tiếp sẽ ra "VERSION_MAJOR")
```

### `##` — nối hai token thành một

```c
#define CONCAT(a, b) a##b
int CONCAT(var, 1) = 10;            // int var1 = 10;

#define DECLARE_GETTER(type, name) \
    type get_##name(const Config *c) { return c->name; }

DECLARE_GETTER(int, port)           // int get_port(const Config *c) { return c->port; }
DECLARE_GETTER(double, timeout)     // double get_timeout(...)
```

Dùng để sinh nhiều hàm/kiểu giống nhau (một dạng "generic" thô sơ trong C, ví dụ tạo `Vec_int`, `Vec_double`):

```c
#define DEFINE_VEC(T)                                         \
    typedef struct { T *data; size_t size, cap; } Vec_##T;    \
    static inline int Vec_##T##_push(Vec_##T *v, T x) {       \
        if (v->size == v->cap) {                              \
            size_t nc = v->cap ? v->cap * 2 : 4;              \
            T *t = realloc(v->data, nc * sizeof *t);          \
            if (!t) return -1;                                \
            v->data = t; v->cap = nc;                         \
        }                                                     \
        v->data[v->size++] = x;                               \
        return 0;                                             \
    }

DEFINE_VEC(int)          // sinh Vec_int và Vec_int_push
DEFINE_VEC(double)       // sinh Vec_double và Vec_double_push
```

Nhược điểm: mã khó đọc, lỗi báo ở dòng macro, khó debug. Dùng có chừng mực.

### Nối dòng bằng `\`

Ký tự `\` ở cuối dòng nối dòng tiếp theo; **không được có khoảng trắng sau `\`**.

## 14.5. Macro biến thiên (variadic)

```c
#define LOG(fmt, ...) fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, __VA_ARGS__)

LOG("x = %d, y = %d", x, y);       // OK
LOG("khoi dong");                  // LỖI: sau fmt còn dấu phẩy thừa vì __VA_ARGS__ rỗng
```

Cách khắc phục:

```c
// gcc/clang (mở rộng): ## nuốt dấu phẩy khi __VA_ARGS__ rỗng
#define LOG(fmt, ...) fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

// C chuẩn: gộp fmt vào __VA_ARGS__
#define LOG2(...) fprintf(stderr, "[%s:%d] " __VA_ARGS__, __FILE__, __LINE__)
```

(C23 chuẩn hóa `__VA_OPT__(,)`.) Macro biến thiên thường dùng cho log và debug.

### Đếm số đối số (kỹ thuật nâng cao)

```c
#define COUNT_ARGS(...) COUNT_ARGS_(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define COUNT_ARGS_(_1, _2, _3, _4, _5, N, ...) N

int n = COUNT_ARGS(a, b, c);    // 3
```

Mẹo này đẩy dãy số về phía sau tùy số đối số thật; hữu ích để "nạp chồng" macro theo số lượng đối số.

## 14.6. X-macro — một danh sách, nhiều sản phẩm

**Vấn đề:** bạn có một danh sách (mã lỗi, lệnh, kiểu token) cần dùng ở **nhiều nơi**: một `enum`, một bảng chuỗi tên, một bảng hàm xử lý. Nếu viết tay từng nơi, thêm một mục phải sửa 3 chỗ và dễ lệch nhau.

**X-macro:** viết danh sách **một lần** dưới dạng macro gọi `X(...)` cho mỗi mục; rồi định nghĩa `X` khác nhau tùy mục đích và "mở rộng" danh sách.

```c
// Danh sách duy nhất
#define ERROR_LIST(X)                       \
    X(ERR_OK,      "thanh cong")            \
    X(ERR_IO,      "loi vao/ra")            \
    X(ERR_OOM,     "het bo nho")            \
    X(ERR_PARSE,   "loi phan tich")

// Sản phẩm 1: enum
#define AS_ENUM(name, text) name,
typedef enum { ERROR_LIST(AS_ENUM) ERR_COUNT } ErrorCode;

// Sản phẩm 2: bảng chuỗi
#define AS_STRING(name, text) [name] = text,
static const char *const ERROR_TEXT[] = { ERROR_LIST(AS_STRING) };

// Sản phẩm 3: bảng tên hằng (để log/debug)
#define AS_NAME(name, text) [name] = #name,
static const char *const ERROR_NAME[] = { ERROR_LIST(AS_NAME) };

const char *error_text(ErrorCode e) {
    return (e >= 0 && e < ERR_COUNT) ? ERROR_TEXT[e] : "?";
}
```

Sau tiền xử lý, `ERROR_LIST(AS_ENUM)` thành `ERR_OK, ERR_IO, ERR_OOM, ERR_PARSE,`; enum, bảng chuỗi và bảng tên luôn đồng bộ. **Thêm một mã lỗi mới chỉ cần thêm một dòng** vào `ERROR_LIST`.

### X-macro cho lệnh CLI

```c
#define COMMANDS(X)                                   \
    X(help,   "hien tro giup")                        \
    X(start,  "khoi dong dich vu")                    \
    X(stop,   "dung dich vu")

// 1) khai báo hàm xử lý
#define DECL(name, desc) static int cmd_##name(int argc, char **argv);
COMMANDS(DECL)

// 2) bảng lệnh
typedef struct { const char *name; const char *desc; int (*fn)(int, char **); } Command;
#define ROW(name, desc) { #name, desc, cmd_##name },
static const Command COMMAND_TABLE[] = { COMMANDS(ROW) };

// 3) cài đặt (mỗi lệnh một hàm)
static int cmd_help(int argc, char **argv)  { (void)argc; (void)argv; puts("help"); return 0; }
static int cmd_start(int argc, char **argv) { (void)argc; (void)argv; puts("start"); return 0; }
static int cmd_stop(int argc, char **argv)  { (void)argc; (void)argv; puts("stop"); return 0; }

int dispatch(const char *name, int argc, char **argv) {
    for (size_t i = 0; i < sizeof COMMAND_TABLE / sizeof COMMAND_TABLE[0]; i++)
        if (strcmp(COMMAND_TABLE[i].name, name) == 0) return COMMAND_TABLE[i].fn(argc, argv);
    fprintf(stderr, "lenh khong ton tai: %s\n", name);
    return 1;
}
```

Thêm lệnh mới: thêm một dòng vào `COMMANDS` và viết hàm `cmd_xxx`. Nếu quên viết hàm, compiler báo lỗi ngay (chưa định nghĩa) — tốt hơn nhiều so với để bảng và enum lệch nhau âm thầm.

## 14.7. Thay thế macro bằng cơ chế tốt hơn

### `static inline` thay macro tính toán

```c
static inline int max_int(int a, int b) { return a > b ? a : b; }
```

An toàn (một lần tính đối số), có kiểm tra kiểu, thấy được trong debugger. Compiler thường chèn thân hàm tại chỗ gọi (inlining) nên **không chậm hơn** macro.

### `enum` thay `#define` cho hằng nguyên

```c
enum { BUFFER_SIZE = 4096, MAX_RETRIES = 3 };
```

### `_Generic` (C11) — chọn theo kiểu

`_Generic(expr, kiểu1: giá_trị1, kiểu2: giá_trị2, default: ...)` chọn biểu thức dựa vào **kiểu** của `expr` lúc biên dịch:

```c
#define ABS(x) _Generic((x),         \
    int:    abs,                     \
    long:   labs,                    \
    double: fabs,                    \
    float:  fabsf                    \
)(x)

int    a = ABS(-5);        // gọi abs
double b = ABS(-2.5);      // gọi fabs
```

```c
#define TYPE_NAME(x) _Generic((x), \
    int: "int", double: "double", char *: "char*", default: "khac")

puts(TYPE_NAME(1));        // int
puts(TYPE_NAME(2.0));      // double
```

Đây là cách C hiện đại tạo "hàm nạp chồng" (như `tgmath.h`).

### Khi nào macro vẫn là lựa chọn đúng?

- Điều kiện biên dịch (`#ifdef`).
- Chuỗi hóa/nối token (`#`, `##`).
- `__FILE__`, `__LINE__`, `__func__` (cần vị trí **nơi gọi**).
- X-macro và sinh mã lặp lại.
- Khai báo macro như `container_of`, `ARRAY_SIZE` cần làm việc với kiểu và tên.

```c
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))   // chỉ dùng cho MẢNG thật, không phải con trỏ
```

## 14.8. Biên dịch có điều kiện

```c
#if defined(DEBUG)
    #define TRACE(...) fprintf(stderr, __VA_ARGS__)
#else
    #define TRACE(...) ((void)0)
#endif
```

### `#if` với biểu thức hằng

```c
#define VERSION 3

#if VERSION >= 3
    /* mã cho v3 trở lên */
#elif VERSION == 2
    /* mã cho v2 */
#else
    #error "phien ban khong ho tro"
#endif
```

`#if` chỉ tính **biểu thức nguyên hằng**: macro chưa định nghĩa được xem là `0`; không dùng được `sizeof`, ép kiểu. `defined(X)` (hoặc `defined X`) trả 1 nếu `X` được định nghĩa.

Tắt tạm một đoạn mã (kể cả có chứa comment): `#if 0 ... #endif`.

### Macro định sẵn (predefined)

| Macro | Ý nghĩa |
|---|---|
| `__FILE__` | tên file nguồn (chuỗi) |
| `__LINE__` | số dòng hiện tại (số nguyên) |
| `__func__` | tên hàm hiện tại (C99; biến chuỗi, không phải macro) |
| `__DATE__`, `__TIME__` | ngày/giờ biên dịch |
| `__STDC_VERSION__` | phiên bản chuẩn (`201112L` cho C11) |
| `__cplusplus` | có định nghĩa nếu biên dịch bằng C++ |
| `__GNUC__` | phiên bản chính của gcc (cũng được clang định nghĩa) |
| `_WIN32`, `__linux__`, `__APPLE__` | nền tảng |
| `__x86_64__`, `__aarch64__` | kiến trúc CPU |
| `NDEBUG` | tắt `assert` nếu được định nghĩa |

Xem toàn bộ macro compiler định nghĩa sẵn: `gcc -dM -E - < /dev/null`.

### Mã cho nhiều nền tảng

```c
#if defined(_WIN32)
    #include <windows.h>
    static void sleep_ms(unsigned ms) { Sleep(ms); }
#elif defined(__unix__) || defined(__APPLE__)
    #include <unistd.h>
    static void sleep_ms(unsigned ms) { usleep(ms * 1000); }
#else
    #error "Nen tang khong duoc ho tro"
#endif
```

Giữ mã phụ thuộc nền tảng **gom vào một chỗ** (một file/một lớp bọc) thay vì rải `#ifdef` khắp nơi — mã sẽ dễ đọc hơn nhiều.

### Feature test macros

Nhiều hàm POSIX (`strdup`, `getline`, `clock_gettime`) chỉ được khai báo khi bạn yêu cầu, đặc biệt với `-std=c11` nghiêm ngặt. Đặt **trước mọi `#include`**:

```c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <time.h>
```

Nếu không, bạn nhận cảnh báo `implicit declaration of function 'strdup'`.

### `#pragma`

```c
#pragma once                      // include guard ngắn gọn (không phải chuẩn nhưng rất phổ biến)
#pragma pack(push, 1)             // bỏ padding trong struct
#pragma pack(pop)
```

C99 còn có `_Pragma("...")` để dùng `#pragma` bên trong macro.

### Include guard và kiểm tra cấu hình

```c
#ifndef CONFIG_H
#define CONFIG_H
#endif
```

`#error` giúp bắt sớm cấu hình sai:

```c
#if !defined(USE_SSL) && !defined(USE_PLAIN)
    #error "Phai dinh nghia USE_SSL hoac USE_PLAIN"
#endif
```

Cũng nên dùng `_Static_assert` (chương 13) cho kiểm tra dựa trên kích thước/kiểu.

## 14.9. Ví dụ hoàn chỉnh: bộ dò lỗi nhẹ

```c
// tiny_test.h — khung kiểm thử tối giản dựa trên macro
#ifndef TINY_TEST_H
#define TINY_TEST_H

#include <stdio.h>

static int tt_run = 0, tt_failed = 0;

#define CHECK(cond) do {                                                     \
        tt_run++;                                                            \
        if (!(cond)) {                                                       \
            tt_failed++;                                                     \
            fprintf(stderr, "%s:%d: CHECK(%s) that bai\n", __FILE__, __LINE__, #cond); \
        }                                                                    \
    } while (0)

#define CHECK_EQ_INT(actual, expected) do {                                  \
        long long _a = (actual), _e = (expected);                            \
        tt_run++;                                                            \
        if (_a != _e) {                                                      \
            tt_failed++;                                                     \
            fprintf(stderr, "%s:%d: %s = %lld, mong doi %lld\n",             \
                    __FILE__, __LINE__, #actual, _a, _e);                    \
        }                                                                    \
    } while (0)

#define TEST_SUMMARY() \
    (printf("%d kiem tra, %d that bai\n", tt_run, tt_failed), tt_failed ? 1 : 0)

#endif
```

```c
// test_math.c
#include "tiny_test.h"

static int add(int a, int b) { return a + b; }

int main(void) {
    CHECK(add(2, 3) == 5);
    CHECK_EQ_INT(add(-1, 1), 0);
    CHECK_EQ_INT(add(2, 2), 5);        // cố tình sai để xem thông báo
    return TEST_SUMMARY();
}
```

Kết quả:

```text
test_math.c:9: add(2, 2) = 4, mong doi 5
3 kiem tra, 1 that bai
```

Ví dụ này dùng đủ kỹ thuật: `do { } while (0)`, `#cond` (chuỗi hóa biểu thức), `__FILE__`/`__LINE__`, biến cục bộ `_a`/`_e` để **đánh giá đối số đúng một lần**. Chương 21 dùng tiếp ý tưởng này.

## 14.10. Lỗi thường gặp

| Lỗi | Ví dụ | Cách tránh |
|---|---|---|
| Thiếu ngoặc | `#define SQ(x) x*x` | `((x)*(x))` |
| Tác dụng phụ bị đánh giá nhiều lần | `MAX(i++, j)` | Dùng `static inline` |
| Macro nhiều câu lệnh không có `do-while` | `#define S(a,b) t=a; a=b;` | `do { ... } while (0)` |
| Thêm `;` hoặc `=` vào `#define` | `#define N = 10;` | `#define N 10` |
| Khoảng trắng sau `\` nối dòng | lỗi khó thấy | Không để khoảng trắng |
| Quên mở rộng macro trước khi chuỗi hóa | `#x` cho ra tên macro | Dùng hai tầng `STR_(x)` |
| Tên macro trùng tên hàm/biến | phá header hệ thống | Đặt CHỮ HOA, thêm tiền tố dự án |
| `#ifdef` rải khắp nơi | mã rối, khó kiểm thử | Gom vào một lớp bọc nền tảng |
| Quên `_POSIX_C_SOURCE` | `implicit declaration` | Định nghĩa trước mọi `#include` |

## 14.11. Tóm tắt

- Bộ tiền xử lý làm việc trên văn bản; dùng `gcc -E` để xem kết quả mở rộng.
- Macro cần ngoặc quanh tham số và cả biểu thức; tránh đối số có tác dụng phụ; dùng `do { } while (0)` cho macro nhiều câu lệnh.
- `#` chuỗi hóa, `##` nối token; macro biến thiên với `__VA_ARGS__`.
- **X-macro** giữ một danh sách nguồn duy nhất để sinh enum, bảng chuỗi, bảng hàm.
- Ưu tiên `static inline`, `enum`, `_Generic`, `const`; giữ macro cho điều kiện biên dịch, chuỗi hóa, vị trí nguồn.
- Biên dịch có điều kiện dựa trên nền tảng/tính năng; gom mã phụ thuộc nền tảng vào một chỗ.

## 14.12. Bài tập

1. Viết macro `SWAP(type, a, b)` an toàn và chứng minh nó dùng đúng trong `if/else` không ngoặc nhọn.
2. Viết `MIN`, `MAX`, `CLAMP(x, lo, hi)` bằng macro; chỉ ra điều gì hỏng với `CLAMP(i++, 0, 10)`; sau đó viết lại bằng `static inline` hoặc `_Generic`.
3. Viết macro `LOG(level, fmt, ...)` xuất `file:line` và tên hàm; so sánh với phiên bản `static inline void log(...)` dùng `va_list` (hàm không tự biết `__LINE__` của nơi gọi — hãy giải thích).
4. Dùng X-macro để sinh cho danh sách trạng thái `{IDLE, RUNNING, PAUSED, STOPPED}`: `enum`, mảng tên chuỗi, và hàm `state_from_string(const char *)`.
5. Dùng X-macro cho danh sách lệnh CLI, tự sinh `switch-case`/bảng phân phối xử lý; thêm một lệnh chỉ cần thêm một dòng.
6. Viết macro `DEFINE_STACK(T)` sinh `Stack_T` với `push`/`pop`; dùng với `int` và `double`. Bàn về ưu/nhược so với `void *`.
7. Viết `#ifdef` để một chương trình in "Windows"/"Linux"/"macOS" và cung cấp hàm `sleep_ms` đa nền tảng.
8. Dùng `gcc -E` trên một chương trình có macro lồng nhau, đọc kết quả và giải thích từng bước mở rộng.
9. (Thử thách) Dùng `_Generic` cài đặt macro `PRINT(x)` in đúng định dạng cho `int`, `double`, `char *`, `char`.

Mã nguồn mẫu: /code/chapter-14
