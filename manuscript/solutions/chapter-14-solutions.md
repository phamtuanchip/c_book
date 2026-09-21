# Chương 14 — Lời giải bài tập

## Bài 1: `SWAP` an toàn

```c
// swap_macro.c
#include <stdio.h>

#define SWAP(type, a, b) do { type swap_tmp__ = (a); (a) = (b); (b) = swap_tmp__; } while (0)

int main(void) {
    int x = 1, y = 2;
    if (x < y)
        SWAP(int, x, y);          // hoạt động như MỘT câu lệnh: if/else không cần { }
    else
        puts("khong doi");
    printf("%d %d\n", x, y);      // 2 1
    return 0;
}
```

Không có `do { } while (0)`, macro nở thành nhiều câu lệnh và `else` sẽ bị lỗi cú pháp (hoặc chỉ câu đầu thuộc `if`). Tên biến tạm `swap_tmp__` khó trùng với biến của người dùng.

## Bài 2: `MIN`, `MAX`, `CLAMP`

```c
// minmax_macro.c
#include <stdio.h>

#define MIN(a, b)        ((a) < (b) ? (a) : (b))
#define MAX(a, b)        ((a) > (b) ? (a) : (b))
#define CLAMP(x, lo, hi) MIN(MAX((x), (lo)), (hi))

static inline int clamp_int(int x, int lo, int hi) { return x < lo ? lo : (x > hi ? hi : x); }

int main(void) {
    int i = 5;
    int a = clamp_int(i++, 0, 10);              // i++ đúng MỘT lần
    printf("clamp_int: %d, i = %d\n", a, i);    // 5, 6
    // CLAMP(i++, 0, 10) sẽ đánh giá i++ tới bốn lần (MIN/MAX mở rộng mỗi tham số nhiều lần) -> UB
    printf("%d\n", CLAMP(20, 0, 10));           // 10 (an toàn vì đối số không có tác dụng phụ)
    return 0;
}
```

Với `_Generic` bạn có thể chọn hàm theo kiểu: `#define CLAMP(x, lo, hi) _Generic((x), int: clamp_int, double: clamp_double)(x, lo, hi)`.

## Bài 3: `LOG` bằng macro và bằng hàm

```c
// log_compare.c
#include <stdarg.h>
#include <stdio.h>

#define LOG(fmt, ...) fprintf(stderr, "[%s:%d %s] " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

static void log_fn(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "[?:? ?] ");           // hàm KHÔNG biết __LINE__ của nơi gọi
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
}

int main(void) {
    LOG("khoi dong");                      // [log_compare.c:19 main] khoi dong
    LOG("x = %d", 42);
    log_fn("x = %d", 42);
    return 0;
}
```

`__FILE__`/`__LINE__`/`__func__` được tính **tại nơi macro được mở rộng** (trong hàm gọi), còn trong hàm `log_fn` chúng chỉ cho vị trí của chính `log_fn`. Đó là lý do log cần macro, dù phần định dạng nên chuyển cho một hàm `vfprintf`.

## Bài 4: X-macro cho trạng thái

```c
// state_xmacro.c
#include <stdio.h>
#include <string.h>

#define STATE_LIST(X) X(IDLE) X(RUNNING) X(PAUSED) X(STOPPED)

typedef enum {
#define X(name) STATE_##name,
    STATE_LIST(X)
#undef X
    STATE_COUNT
} State;

static const char *const STATE_NAMES[] = {
#define X(name) [STATE_##name] = #name,
    STATE_LIST(X)
#undef X
};

static const char *state_name(State s) { return (s >= 0 && s < STATE_COUNT) ? STATE_NAMES[s] : "?"; }

/* Trả STATE_COUNT nếu không nhận ra */
static State state_from_string(const char *s) {
    for (int i = 0; i < STATE_COUNT; i++)
        if (strcmp(STATE_NAMES[i], s) == 0) return (State)i;
    return STATE_COUNT;
}

int main(void) {
    printf("%s %d\n", state_name(STATE_PAUSED), (int)state_from_string("RUNNING"));   // PAUSED 1
    return 0;
}
```

Thêm trạng thái mới chỉ cần thêm `X(NAME)` vào `STATE_LIST`; enum, bảng tên và hàm tra cứu tự cập nhật.

## Bài 5: X-macro cho lệnh CLI

Xem ví dụ `COMMANDS(X)` trong mục 14.6: thêm một lệnh = thêm một dòng vào danh sách và viết hàm `cmd_xxx`; nếu quên viết hàm, linker báo `undefined reference` — lỗi được phát hiện sớm thay vì để bảng và enum lệch nhau.

## Bài 6: `DEFINE_STACK(T)`

```c
// define_stack.c
#include <stdio.h>
#include <stdlib.h>

#define DEFINE_STACK(T)                                                     \
    typedef struct { T *data; size_t size, cap; } Stack_##T;                \
    static int Stack_##T##_push(Stack_##T *s, T v) {                        \
        if (s->size == s->cap) {                                            \
            size_t nc = s->cap ? s->cap * 2 : 4;                            \
            T *t = realloc(s->data, nc * sizeof *t);                        \
            if (!t) return -1;                                              \
            s->data = t; s->cap = nc;                                       \
        }                                                                   \
        s->data[s->size++] = v;                                             \
        return 0;                                                           \
    }                                                                       \
    static int Stack_##T##_pop(Stack_##T *s, T *out) {                      \
        if (s->size == 0) return -1;                                        \
        *out = s->data[--s->size];                                          \
        return 0;                                                           \
    }

DEFINE_STACK(int)
DEFINE_STACK(double)

int main(void) {
    Stack_int si = {0};
    Stack_double sd = {0};
    Stack_int_push(&si, 7);
    Stack_double_push(&sd, 2.5);
    int a; double b;
    Stack_int_pop(&si, &a);
    Stack_double_pop(&sd, &b);
    printf("%d %.1f\n", a, b);
    free(si.data); free(sd.data);
    return 0;
}
```

So với `void *`: macro cho **an toàn kiểu** và không phải ép kiểu/cấp phát riêng từng phần tử (hiệu năng tốt), nhưng lỗi biên dịch khó đọc và mã lặp trong file thực thi; `void *` gọn hơn nhưng mất kiểm tra kiểu và phải quản lý kích thước phần tử.

## Bài 7: `sleep_ms` đa nền tảng và in tên hệ điều hành

```c
// platform.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>

#if defined(_WIN32)
  #include <windows.h>
  static void sleep_ms(unsigned ms) { Sleep(ms); }
  #define OS_NAME "Windows"
#elif defined(__unix__) || defined(__APPLE__)
  #include <time.h>
  static void sleep_ms(unsigned ms) {
      struct timespec ts = { ms / 1000, (long)(ms % 1000) * 1000000L };
      nanosleep(&ts, NULL);
  }
  #if defined(__APPLE__)
    #define OS_NAME "macOS"
  #else
    #define OS_NAME "Linux/Unix"
  #endif
#else
  #error "nen tang khong duoc ho tro"
#endif

int main(void) {
    printf("chay tren %s\n", OS_NAME);
    sleep_ms(100);
    return 0;
}
```

## Bài 8: đọc kết quả `gcc -E`

Ví dụ: với `#define A(x) B(x) + 1` và `#define B(x) ((x) * 2)`, dòng `int r = A(3);` được nở như sau: (1) `A(3)` → `B(3) + 1`; (2) bộ tiền xử lý quét lại kết quả và nở `B(3)` → `((3) * 2)`; (3) kết quả: `int r = ((3) * 2) + 1;`. Chạy `gcc -E prog.c | tail` để thấy dòng cuối cùng này. Macro **không** đệ quy: tên macro đang được nở không được nở lại bên trong chính nó.

## Bài 9: `PRINT(x)` bằng `_Generic`

```c
// generic_print.c
#include <stdio.h>

static void print_int(int x)          { printf("int: %d\n", x); }
static void print_long(long x)        { printf("long: %ld\n", x); }
static void print_double(double x)    { printf("double: %g\n", x); }
static void print_char(char x)        { printf("char: %c\n", x); }
static void print_str(const char *x)  { printf("string: %s\n", x); }

/* Chọn HÀM theo kiểu rồi mới gọi: mỗi nhánh chỉ là tên hàm nên không có cảnh báo định dạng ở nhánh không được chọn */
#define PRINT(x) _Generic((x),   \
    int:          print_int,     \
    long:         print_long,    \
    double:       print_double,  \
    char:         print_char,    \
    char *:       print_str,     \
    const char *: print_str)(x)

int main(void) {
    PRINT(42);
    PRINT(3.5);
    PRINT('A');                         // lưu ý: 'A' có kiểu int trong C, nên in "int: 65"
    PRINT("xin chao");                  // "xin chao" phân rã thành char * trong _Generic của gcc/clang
    return 0;
}
```

Điều đáng nhớ: trong C, `'A'` có kiểu **`int`**, không phải `char` (khác C++), nên nhánh `char:` chỉ được chọn với biến `char`. Kiểm tra bằng `char c = 'A'; PRINT(c);`.
