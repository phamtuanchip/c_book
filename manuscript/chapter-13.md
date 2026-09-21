# Chương 13 — Xử lý lỗi & ngoại lệ trong C

## Mục tiêu chương

- Hiểu vì sao C **không có ngoại lệ** và các chiến lược thay thế.
- Phân biệt **lỗi lập trình (bug)** với **lỗi vận hành (runtime error)**, và biết xử lý từng loại thế nào.
- Dùng đúng giá trị trả về, `errno`, `perror`, `strerror`, `assert`.
- Thiết kế API báo lỗi rõ ràng: mã lỗi, `enum`, tham số đầu ra, struct kết quả.
- Áp dụng mẫu **dọn dẹp bằng `goto cleanup`** để không rò rỉ tài nguyên.
- Dùng `setjmp`/`longjmp` (và biết vì sao nên hạn chế), `atexit`, tín hiệu.
- Xây dựng hệ thống **logging** bằng macro biến thiên (`__VA_ARGS__`, `__FILE__`, `__LINE__`).
- Kiểm thử đường lỗi bằng **fault injection**.

## 13.1. C không có ngoại lệ

Java, Python, C++ có `try/catch`: khi lỗi xảy ra, chương trình "ném" ngoại lệ và luồng điều khiển tự nhảy tới nơi xử lý. **C không có cơ chế đó.** Thay vào đó, mỗi hàm có thể thất bại **phải tự báo** cho người gọi biết, và người gọi **phải chủ động kiểm tra**.

Hệ quả: **lỗi bị bỏ qua thì không ai báo cho bạn.** Chương trình cứ chạy tiếp với dữ liệu sai, rồi hỏng ở chỗ khác — khó tìm nguyên nhân. Kỷ luật quan trọng nhất của lập trình C là:

> **Mọi hàm có thể thất bại đều phải được kiểm tra kết quả.**

### Hai loại lỗi cần phân biệt

| | Lỗi lập trình (bug) | Lỗi vận hành (runtime error) |
|---|---|---|
| Ví dụ | Truyền `NULL` cho hàm không cho phép NULL; chỉ số ngoài mảng; vi phạm bất biến | File không tồn tại; mạng đứt; hết bộ nhớ; đầu vào người dùng sai |
| Có thể tránh bằng cách viết đúng? | **Có** | **Không** — xảy ra do môi trường |
| Xử lý | `assert`, sửa mã | Kiểm tra và báo lỗi/khôi phục |
| Trong bản phát hành | Không nên xảy ra | Phải xử lý êm |

Đừng dùng cùng một cơ chế cho hai loại: `assert` cho bug, mã lỗi cho runtime error.

## 13.2. Giá trị trả về và `errno`

### Các quy ước trả về lỗi của thư viện chuẩn

| Hàm | Dấu hiệu lỗi |
|---|---|
| `malloc`, `calloc`, `realloc` | trả `NULL` |
| `fopen` | trả `NULL` (đặt `errno`) |
| `fgets` | trả `NULL` |
| `getchar`, `fgetc` | trả `EOF` |
| `scanf` | trả số mục đọc ít hơn mong đợi |
| `printf` | trả số âm |
| `strtol` | `errno == ERANGE` hoặc `end == s` |
| Nhiều lời gọi hệ thống POSIX (`open`, `read`, `write`) | trả `-1` (đặt `errno`) |

### `errno`

`errno` (trong `<errno.h>`) là biến toàn cục (thực chất là macro) do hàm thư viện/hệ thống đặt khi thất bại. Các quy tắc bắt buộc nhớ:

1. **Chỉ đọc `errno` ngay sau một lời gọi vừa báo lỗi.** Hàm **thành công** *không* đặt lại `errno` về 0, nên `errno` có thể còn "rác" từ lần trước.
2. Nếu cần chắc chắn (như với `strtol`), **gán `errno = 0` trước khi gọi**.
3. Lời gọi khác giữa chừng (kể cả `printf`) có thể đổi `errno` → lưu lại ngay: `int saved = errno;`.
4. Với đa luồng, `errno` là riêng của từng luồng.

Một số giá trị thường gặp (định nghĩa trong `<errno.h>`):

| Hằng | Ý nghĩa |
|---|---|
| `ENOENT` | file/thư mục không tồn tại |
| `EACCES` | không đủ quyền |
| `ENOMEM` | hết bộ nhớ |
| `EINVAL` | đối số không hợp lệ |
| `EEXIST` | đã tồn tại |
| `ERANGE` | kết quả ngoài phạm vi (ví dụ `strtol`) |
| `EINTR` | bị gián đoạn bởi tín hiệu |

### In thông báo lỗi có ý nghĩa

```c
#include <errno.h>
#include <stdio.h>
#include <string.h>

FILE *f = fopen(path, "r");
if (f == NULL) {
    int saved = errno;                                        // lưu lại ngay
    fprintf(stderr, "Khong mo duoc '%s': %s (errno=%d)\n", path, strerror(saved), saved);
    return -1;
}

// Cách gọn: perror in "tiền tố: thông báo" ra stderr
perror("fopen");
```

`strerror` trả về con trỏ tới bộ đệm tĩnh nên **không an toàn với đa luồng**; dùng `strerror_r` (POSIX) hoặc `strerror_s` (C11 Annex K / Windows) khi cần.

**Thông báo lỗi tốt** trả lời ba câu hỏi: *cái gì hỏng?* (thao tác), *với cái gì?* (đối tượng: tên file, giá trị), *vì sao?* (`strerror`). So sánh:

```text
Xấu:  Error
Tốt:  Khong mo duoc 'config.ini' de doc: No such file or directory
```

### Ví dụ: `strtol` với kiểm tra đầy đủ

```c
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

// Trả về 0 nếu đổi thành công; -1 nếu lỗi (chuỗi rỗng, có ký tự thừa, hoặc tràn)
int parse_long(const char *s, long *out) {
    char *end;
    errno = 0;                                   // xóa trước khi gọi
    long v = strtol(s, &end, 10);
    if (end == s || strpbrk(s, "0123456789") == NULL) return -1;   // không có chữ số nào
    if (*end != '\0') return -1;                 // còn ký tự lạ ("12abc")
    if (errno == ERANGE) return -1;              // ngoài phạm vi long
    *out = v;
    return 0;
}
```

## 13.3. Thiết kế API báo lỗi

Có nhiều cách để hàm báo kết quả, tùy tình huống. Hãy chọn một quy ước và **nhất quán trong toàn dự án**.

### Cách 1: trả về mã trạng thái, kết quả qua tham số đầu ra

```c
typedef enum {
    ERR_OK = 0,
    ERR_IO,
    ERR_OOM,
    ERR_PARSE,
    ERR_INVALID_ARG
} ErrorCode;

const char *error_string(ErrorCode e) {
    switch (e) {
        case ERR_OK:          return "thanh cong";
        case ERR_IO:          return "loi vao/ra";
        case ERR_OOM:         return "het bo nho";
        case ERR_PARSE:       return "loi phan tich";
        case ERR_INVALID_ARG: return "doi so khong hop le";
    }
    return "loi khong ro";
}

ErrorCode config_load(const char *path, Config **out);
```

Người gọi:

```c
Config *cfg = NULL;
ErrorCode e = config_load("app.conf", &cfg);
if (e != ERR_OK) {
    fprintf(stderr, "Tai cau hinh that bai: %s\n", error_string(e));
    return 1;
}
```

Ưu điểm: phân biệt được nhiều loại lỗi; giá trị "kết quả" và "lỗi" tách bạch (khắc phục vấn đề "không biết `-1` là kết quả hay lỗi").

Tránh trùng tên: `error_t` đã là kiểu do POSIX/glibc định nghĩa; hãy đặt tên như `ErrorCode` hoặc thêm tiền tố dự án (`myapp_err_t`).

### Cách 2: trả về con trỏ, `NULL` là lỗi

```c
char *read_file(const char *path);     // NULL nếu lỗi; nếu cần lý do, dùng errno hoặc tham số phụ
```

Phù hợp khi chỉ có "thành công/thất bại"; nếu cần lý do, thêm tham số `ErrorCode *err` hoặc để `errno` mang lý do.

### Cách 3: giá trị trả về đặc biệt (sentinel)

`-1`, `NULL`, `EOF`. Chỉ dùng khi **giá trị đặc biệt không thể là kết quả hợp lệ**. Hàm trả về `int` mà `-1` cũng có thể là kết quả đúng thì không dùng được cách này.

### Cách 4: struct kết quả

```c
typedef struct {
    int    ok;              // 1 nếu thành công
    double value;           // hợp lệ khi ok
    char   message[64];     // mô tả lỗi khi !ok
} DivResult;

DivResult safe_divide(double a, double b) {
    DivResult r = {0};
    if (b == 0.0) {
        snprintf(r.message, sizeof r.message, "chia cho 0");
        return r;
    }
    r.ok = 1;
    r.value = a / b;
    return r;
}
```

### Cách 5: bộ đệm thông báo lỗi do người gọi cung cấp

```c
// Trả NULL nếu lỗi và ghi mô tả (có nghĩa với người dùng) vào err_buf
FILE *safe_fopen(const char *path, const char *mode, char *err_buf, size_t err_size) {
    FILE *f = fopen(path, mode);
    if (f == NULL) {
        int saved = errno;
        if (err_buf && err_size > 0)
            snprintf(err_buf, err_size, "khong mo duoc '%s' (%s): %s", path, mode, strerror(saved));
        errno = saved;              // giữ lại errno cho người gọi
        return NULL;
    }
    if (err_buf && err_size > 0) err_buf[0] = '\0';
    return f;
}
```

Ưu điểm: không dùng bộ nhớ tĩnh (an toàn đa luồng), không cần người gọi `free`.

### Nguyên tắc thiết kế

1. **Hàm phải nói rõ có thể thất bại thế nào** trong tài liệu (ghi chú trên prototype).
2. **Không trả kết quả một phần** khi lỗi: nếu thất bại, để trạng thái đầu ra **nguyên vẹn hoặc rõ ràng không hợp lệ** (ví dụ `*out` không bị đổi).
3. **Không im lặng nuốt lỗi**. Nếu hàm xử lý được lỗi thì xử lý; nếu không thì trả lên trên.
4. **Ở tầng thấp: trả lỗi. Ở tầng cao (`main`): quyết định làm gì** (in thông báo, thử lại, thoát).
5. **Đừng gọi `exit()` sâu trong thư viện** — nó tước quyền quyết định của người dùng thư viện. Chỉ `main` (hoặc lớp ứng dụng) nên thoát.
6. Dùng `[[nodiscard]]`-tương đương: gcc/clang có `__attribute__((warn_unused_result))` để compiler cảnh báo khi người gọi bỏ qua giá trị trả về; C23 có `[[nodiscard]]`.

```c
#if defined(__GNUC__)
#  define MUST_CHECK __attribute__((warn_unused_result))
#else
#  define MUST_CHECK
#endif

MUST_CHECK ErrorCode config_load(const char *path, Config **out);
```

## 13.4. `assert` — kiểm tra bất biến của lập trình viên

```c
#include <assert.h>

void list_insert(List *l, size_t index, int value) {
    assert(l != NULL);                       // bất biến: người gọi KHÔNG được truyền NULL
    assert(index <= l->size);                // bất biến: chỉ số hợp lệ
    /* ... */
}
```

`assert(cond)` kiểm tra `cond`; nếu **sai**, in `file:line: function: Assertion 'cond' failed.` rồi gọi `abort()`. Với biên dịch có `-DNDEBUG`, mọi `assert` **bị xóa hoàn toàn**.

Điều đó dẫn tới các quy tắc:

1. **Dùng `assert` cho bug** (giả định nội bộ phải luôn đúng), **không** cho lỗi vận hành.

```c
assert(f != NULL);          // SAI: fopen thất bại là chuyện bình thường, phải kiểm tra bằng if
if (f == NULL) { ... }      // ĐÚNG
```

2. **Không đặt tác dụng phụ trong `assert`** — nó biến mất khi `NDEBUG`:

```c
assert(fread(buf, 1, n, f) == n);      // SAI: bản release không đọc file nữa!
size_t got = fread(buf, 1, n, f);      // ĐÚNG: thực hiện rồi mới assert
assert(got == n);
```

3. Dữ liệu từ ngoài (người dùng, file, mạng) **luôn** phải kiểm tra bằng `if`, không bằng `assert`.

**`static_assert`** (C11, `<assert.h>` hoặc `_Static_assert`) kiểm tra **lúc biên dịch**:

```c
_Static_assert(sizeof(int) == 4, "code nay gia dinh int 4 byte");
_Static_assert(sizeof(Header) == 16, "Header phai dung 16 byte");
```

## 13.5. Dọn dẹp tài nguyên với `goto cleanup`

Khi hàm lấy nhiều tài nguyên (bộ nhớ, file, khóa), mỗi đường lỗi phải giải phóng những gì **đã** lấy. Viết bằng `if` lồng nhau rất rối và dễ sót. Mẫu `goto cleanup` giải quyết gọn:

```c
ErrorCode process_file(const char *in_path, const char *out_path) {
    ErrorCode rc = ERR_OK;
    FILE *in = NULL, *out = NULL;
    char *buf = NULL;

    in = fopen(in_path, "rb");
    if (!in)  { rc = ERR_IO;  goto cleanup; }

    out = fopen(out_path, "wb");
    if (!out) { rc = ERR_IO;  goto cleanup; }

    buf = malloc(64 * 1024);
    if (!buf) { rc = ERR_OOM; goto cleanup; }

    size_t n;
    while ((n = fread(buf, 1, 64 * 1024, in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) { rc = ERR_IO; goto cleanup; }
    }
    if (ferror(in)) rc = ERR_IO;

cleanup:                        // một điểm dọn dẹp duy nhất
    free(buf);                  // free(NULL) an toàn
    if (out) { if (fclose(out) != 0 && rc == ERR_OK) rc = ERR_IO; }
    if (in)  fclose(in);
    return rc;
}
```

Bí quyết: **khởi tạo mọi tài nguyên bằng `NULL`** ở đầu hàm để đoạn `cleanup` chỉ giải phóng những cái thật sự đã lấy. `goto` ở đây là công cụ chấp nhận được (dùng trong nhân Linux); quy tắc: chỉ nhảy **về phía trước**, tới **một nhãn dọn dẹp**.

### Dọn dẹp tự động (GCC/Clang)

Mở rộng `__attribute__((cleanup(fn)))` gọi `fn` khi biến ra khỏi phạm vi (giống RAII/`defer`), nhưng không thuộc chuẩn C:

```c
static void free_ptr(void *p) { free(*(void **)p); }

void f(void) {
    __attribute__((cleanup(free_ptr))) char *buf = malloc(100);
    if (!buf) return;
    /* buf tự được free khi f kết thúc, dù thoát ở đường nào */
}
```

## 13.6. `setjmp` / `longjmp` — nhảy không cục bộ

`setjmp`/`longjmp` (`<setjmp.h>`) cho phép **nhảy ngược ra khỏi nhiều tầng hàm**, gần giống ném/bắt ngoại lệ:

```c
#include <setjmp.h>
#include <stdio.h>

static jmp_buf on_error;

static void deep(int n) {
    if (n == 0) longjmp(on_error, 1);      // "ném": nhảy về nơi setjmp, trả về 1
    deep(n - 1);
}

int main(void) {
    if (setjmp(on_error) == 0) {           // lần đầu: trả về 0 -> đường "try"
        printf("bat dau\n");
        deep(5);
        printf("khong toi day\n");
    } else {                               // sau longjmp: trả về khác 0 -> đường "catch"
        printf("da nhay ve tu ham sau\n");
    }
    return 0;
}
```

Nhưng nó rất **nguy hiểm**:

- **Không giải phóng gì cả.** Mọi bộ nhớ/file lấy ở các tầng bị bỏ qua đều **rò rỉ**.
- Giá trị các biến cục bộ thay đổi sau `setjmp` mà không phải `volatile` là **không xác định**.
- Không được `longjmp` vào hàm đã kết thúc.

Chỉ nên dùng ở những nơi hạn chế và có lý do rõ ràng (thư viện phân tích lớn, bộ thông dịch). Với đa số chương trình, **`goto cleanup` + mã lỗi** là đủ và an toàn hơn.

## 13.7. Thoát chương trình và dọn dẹp toàn cục

```c
#include <stdlib.h>

void exit(int status);         // kết thúc êm: chạy hàm atexit, xả và đóng mọi FILE
void abort(void);              // kết thúc đột ngột (SIGABRT), có thể tạo core dump
int  atexit(void (*fn)(void)); // đăng ký hàm chạy khi exit()/return từ main
```

```c
static FILE *g_log;
static void close_log(void) { if (g_log) fclose(g_log); }

int main(void) {
    g_log = fopen("app.log", "a");
    atexit(close_log);         // đảm bảo đóng file kể cả khi có nơi khác gọi exit()
    /* ... */
    return 0;
}
```

- `return n` trong `main` tương đương `exit(n)`.
- `_Exit(status)` thoát ngay, **không** chạy `atexit` hay xả bộ đệm — dùng trong tiến trình con sau `fork`.
- Mã thoát: `EXIT_SUCCESS` (0), `EXIT_FAILURE` (1).

## 13.8. Tín hiệu (signal) — lỗi từ hệ điều hành

Một số lỗi nghiêm trọng khiến hệ điều hành gửi **tín hiệu** cho tiến trình:

| Tín hiệu | Nguyên nhân | Mặc định |
|---|---|---|
| `SIGSEGV` | truy cập bộ nhớ không hợp lệ | dừng, "Segmentation fault" |
| `SIGFPE` | lỗi số học (chia nguyên cho 0) | dừng |
| `SIGABRT` | `abort()`/`assert` thất bại | dừng |
| `SIGINT` | Ctrl+C | dừng |
| `SIGTERM` | yêu cầu kết thúc | dừng |

Bạn có thể đăng ký hàm xử lý bằng `signal` (chuẩn C) hoặc `sigaction` (POSIX, đáng tin hơn):

```c
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static volatile sig_atomic_t g_stop = 0;

static void on_sigint(int sig) {
    (void)sig;
    g_stop = 1;                    // chỉ đặt cờ; KHÔNG làm việc phức tạp trong handler
}

int main(void) {
    signal(SIGINT, on_sigint);
    while (!g_stop) {
        /* làm việc... */
    }
    printf("nhan Ctrl+C, dung lai va don dep\n");
    return 0;
}
```

Quy tắc an toàn: trong signal handler chỉ nên **đặt cờ kiểu `volatile sig_atomic_t`** hoặc gọi các hàm "an toàn với tín hiệu"; **không** `printf`, `malloc`, `free`. Không nên "khôi phục" sau `SIGSEGV` — trạng thái chương trình đã hỏng; hãy chỉ ghi log nhanh rồi thoát. Chương 18 dùng lại khái niệm này.

## 13.9. Logging

Ghi log giúp chẩn đoán lỗi ở nơi không có debugger (máy khách, server). Một hệ thống log tối thiểu dùng **macro biến thiên** (variadic macro):

```c
// log.h
#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <time.h>

typedef enum { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR } LogLevel;

extern LogLevel g_log_level;              // định nghĩa trong log.c; mức tối thiểu được ghi

void log_write(LogLevel lvl, const char *file, int line, const char *fmt, ...)
#if defined(__GNUC__)
    __attribute__((format(printf, 4, 5)))  // để compiler kiểm tra định dạng như printf
#endif
;

#define LOG_DEBUG_MSG(...) log_write(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO_MSG(...)  log_write(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN_MSG(...)  log_write(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERR_MSG(...)   log_write(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#endif
```

```c
// log.c
#define _POSIX_C_SOURCE 200809L        // để có localtime_r
#include "log.h"
#include <stdarg.h>

LogLevel g_log_level = LOG_INFO;

void log_write(LogLevel lvl, const char *file, int line, const char *fmt, ...) {
    if (lvl < g_log_level) return;

    static const char *names[] = {"DEBUG", "INFO", "WARN", "ERROR"};
    time_t now = time(NULL);
    struct tm tmv;
#if defined(_WIN32)
    localtime_s(&tmv, &now);
#else
    localtime_r(&now, &tmv);
#endif
    char ts[32];
    strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", &tmv);

    fprintf(stderr, "%s [%s] %s:%d: ", ts, names[lvl], file, line);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);          // v-phiên bản nhận va_list
    va_end(ap);
    fputc('\n', stderr);
}
```

Dùng:

```c
LOG_INFO_MSG("mo file %s", path);
LOG_ERR_MSG("khong doc duoc %s: %s", path, strerror(errno));
```

Ghi chú về macro:

- `__FILE__`, `__LINE__` là macro có sẵn: tên file và số dòng **nơi macro được dùng** (vì vậy phải là macro, không phải hàm).
- `__VA_ARGS__` chuyển tiếp mọi đối số. Gắn `##__VA_ARGS__` (mở rộng của gcc/clang) cho phép gọi macro không có đối số biến thiên: `#define LOG(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)`.
- `__func__` (C99) cho tên hàm hiện tại.

**Quy tắc log:**

- Ghi log ra **stderr** hoặc file, không lẫn với đầu ra chương trình (stdout).
- Log **cái gì hỏng và ngữ cảnh** (id, đường dẫn, mã lỗi), nhưng **không log dữ liệu nhạy cảm** (mật khẩu, khóa, thẻ).
- Có **mức** (DEBUG/INFO/WARN/ERROR) điều chỉnh được lúc chạy hoặc lúc biên dịch.
- Log lỗi **một lần, ở nơi xử lý nó**, đừng vừa log vừa trả lỗi rồi tầng trên lại log tiếp (trùng lặp).

## 13.10. Kiểm thử đường lỗi — fault injection

Mã xử lý lỗi hiếm khi chạy nên thường chứa bug (lỗi trong chính đoạn xử lý lỗi!). Để kiểm thử, **cố ý làm cho lệnh thất bại**.

### Bọc hàm để chèn lỗi

```c
// fault.h
#ifdef FAULT_INJECTION
extern int g_fail_after;               // số lần gọi thành công trước khi cấp phát bắt đầu lỗi
void *test_malloc(size_t n);
#define MALLOC(n) test_malloc(n)
#else
#define MALLOC(n) malloc(n)
#endif
```

```c
// fault.c
#include <stdlib.h>
#include "fault.h"
int g_fail_after = -1;                 // -1: không chèn lỗi

void *test_malloc(size_t n) {
    if (g_fail_after == 0) return NULL;          // giả lập hết bộ nhớ
    if (g_fail_after > 0) g_fail_after--;
    return malloc(n);
}
```

Trong bài kiểm thử, lặp qua `g_fail_after = 0, 1, 2, ...` và kiểm tra rằng hàm cần thử **luôn trả lỗi đúng và không rò rỉ** (chạy dưới ASan/Valgrind) ở mọi điểm cấp phát có thể thất bại.

### Các cách khác

- **Truyền đường dẫn không tồn tại, file rỗng, file bị cắt cụt, file quá lớn** để kiểm tra đường I/O.
- Dùng `ulimit -v` (giới hạn bộ nhớ) hoặc `ulimit -n` (giới hạn file mở) để gây lỗi thật.
- Dùng công cụ như `LD_PRELOAD` chèn lỗi, hoặc fuzzer (chương 18, 21).

## 13.11. Ví dụ tổng hợp

Chương trình đọc số nguyên từ file, tính tổng, xử lý lỗi đầy đủ:

```c
// sum_file.c
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { SUM_OK = 0, SUM_ERR_IO, SUM_ERR_PARSE, SUM_ERR_OVERFLOW } SumError;

static const char *sum_error_str(SumError e) {
    switch (e) {
        case SUM_OK:           return "thanh cong";
        case SUM_ERR_IO:       return "loi doc file";
        case SUM_ERR_PARSE:    return "dong khong phai so nguyen";
        case SUM_ERR_OVERFLOW: return "tong bi tran";
    }
    return "?";
}

// Tính tổng các dòng trong file. *bad_line trả về số dòng lỗi (nếu có).
static SumError sum_file(const char *path, long *out_sum, unsigned *bad_line) {
    FILE *f = fopen(path, "r");
    if (!f) return SUM_ERR_IO;

    SumError rc = SUM_OK;
    long sum = 0;
    char line[128];
    unsigned lineno = 0;

    while (fgets(line, sizeof line, f)) {
        lineno++;
        char *end;
        errno = 0;
        long v = strtol(line, &end, 10);
        if (end == line || (*end != '\n' && *end != '\0') || errno == ERANGE) {
            rc = SUM_ERR_PARSE; *bad_line = lineno; goto done;
        }
        if ((v > 0 && sum > LONG_MAX - v) || (v < 0 && sum < LONG_MIN - v)) {
            rc = SUM_ERR_OVERFLOW; *bad_line = lineno; goto done;
        }
        sum += v;
    }
    if (ferror(f)) rc = SUM_ERR_IO;
    else *out_sum = sum;

done:
    fclose(f);
    return rc;
}

int main(int argc, char *argv[]) {
    if (argc != 2) { fprintf(stderr, "Cach dung: %s <file>\n", argv[0]); return 2; }

    long sum = 0;
    unsigned bad = 0;
    SumError e = sum_file(argv[1], &sum, &bad);
    if (e != SUM_OK) {
        if (e == SUM_ERR_IO)
            fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], strerror(errno));
        else
            fprintf(stderr, "%s:%u: %s\n", argv[1], bad, sum_error_str(e));
        return 1;
    }
    printf("tong = %ld\n", sum);
    return 0;
}
```

Chú ý: kiểm tra tràn **trước** khi cộng (`sum > LONG_MAX - v`) vì tràn số nguyên có dấu là UB; lỗi được trả lên `main`, nơi duy nhất quyết định in gì và thoát mã nào; tài nguyên (`FILE *`) được đóng ở một chỗ (`done`).

## 13.12. Lỗi thường gặp

| Lỗi | Hậu quả | Cách tránh |
|---|---|---|
| Không kiểm tra giá trị trả về | Lỗi im lặng, hỏng dây chuyền | Kiểm tra mọi hàm có thể lỗi; bật `warn_unused_result` |
| Đọc `errno` khi không có lỗi | Thông báo sai | Chỉ đọc ngay sau lời gọi vừa lỗi |
| `errno` bị ghi đè bởi `printf` giữa chừng | Sai mã lỗi | Lưu `errno` ngay vào biến |
| `assert` cho dữ liệu vào từ ngoài | Bản release không kiểm tra | Dùng `if` |
| Tác dụng phụ trong `assert` | Hành vi khác giữa debug/release | Tách ra ngoài |
| `exit()` sâu trong thư viện | Người dùng mất quyền xử lý | Trả lỗi lên trên |
| Rò rỉ trên đường lỗi | Rò rỉ tài nguyên | `goto cleanup`, khởi tạo con trỏ NULL |
| Lạm dụng `setjmp/longjmp` | Rò rỉ, trạng thái hỏng | Dùng mã lỗi |
| `printf` trong signal handler | Deadlock/UB | Chỉ đặt cờ `sig_atomic_t` |
| Log dữ liệu nhạy cảm | Rò rỉ bí mật | Không log mật khẩu/khóa/token |

## 13.13. Tóm tắt

- C không có ngoại lệ: **hàm báo lỗi qua giá trị trả về, `errno`, tham số đầu ra**; người gọi phải kiểm tra.
- Phân biệt **bug** (dùng `assert`, sửa mã) và **lỗi vận hành** (kiểm tra, báo lỗi, khôi phục).
- Thiết kế API báo lỗi nhất quán: `enum` mã lỗi, `NULL` cho con trỏ, struct kết quả; ghi tài liệu rõ ràng.
- Dùng **`goto cleanup`** với tài nguyên khởi tạo `NULL` để dọn dẹp trên mọi đường thoát.
- Log có mức, có ngữ cảnh (`__FILE__`, `__LINE__`), ra `stderr`; không log dữ liệu nhạy cảm.
- Kiểm thử đường lỗi bằng fault injection và sanitizer.

## 13.14. Bài tập

1. Viết `safe_fopen(path, mode, err_buf, size)` như 13.3 và chương trình thử với file không tồn tại, thư mục, file không có quyền. In thông báo thân thiện.
2. Viết `parse_int(const char *s, int *out)` bắt được đầu vào rỗng, ký tự thừa, tràn `int`. Viết bộ kiểm thử với ít nhất 10 trường hợp biên (`""`, `" 12"`, `"12 "`, `"2147483648"`, `"-2147483648"`...).
3. Viết lại một chương trình bất kỳ ở chương 12 để dùng `goto cleanup` và `ErrorCode`, đảm bảo Valgrind/ASan không báo rò rỉ trên mọi đường lỗi.
4. Cài đặt `log.h`/`log.c` như trên; thêm ghi ra file, mức log đặt qua biến môi trường `LOG_LEVEL`.
5. Thêm fault injection vào `malloc` của `Vec` (chương 9); viết vòng lặp kiểm thử chứng minh `vec_push` không rò rỉ và không hỏng khi `malloc` lỗi ở bất kỳ lần nào.
6. Viết chương trình xử lý `SIGINT`: khi người dùng nhấn Ctrl+C lần đầu thì thông báo và thoát êm (đóng file, xả bộ đệm), lần hai thoát ngay.
7. Viết macro `CHECK(expr)` in `file:line` và `strerror(errno)` rồi `goto cleanup` khi `expr` là lỗi, và dùng nó để rút gọn mã của bài 3.
8. (Thử thách) Cài đặt cơ chế "try/catch" giả bằng `setjmp`/`longjmp` có ngăn xếp các `jmp_buf`, sau đó liệt kê các cách nó có thể rò rỉ tài nguyên và cách bạn sẽ tránh.

Mã nguồn mẫu: /code/chapter-13
