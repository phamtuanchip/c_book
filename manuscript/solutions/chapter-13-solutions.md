# Chương 13 — Lời giải bài tập

## Bài 1: `safe_fopen` và thử với các lỗi

`safe_fopen` ở mục 13.3 (cách 5). Chương trình thử:

```c
// try_fopen.c
#include <errno.h>
#include <stdio.h>
#include <string.h>

static FILE *safe_fopen(const char *path, const char *mode, char *err, size_t cap) {
    FILE *f = fopen(path, mode);
    if (!f) {
        int saved = errno;
        snprintf(err, cap, "khong mo duoc '%s' (%s): %s", path, mode, strerror(saved));
        errno = saved;
        return NULL;
    }
    err[0] = '\0';
    return f;
}

int main(void) {
    const char *paths[] = { "khong_ton_tai.txt", ".", "/etc/shadow" };   // không có file / là thư mục / không có quyền
    char err[200];
    for (size_t i = 0; i < 3; i++) {
        FILE *f = safe_fopen(paths[i], "r", err, sizeof err);
        if (!f) printf("loi: %s\n", err);
        else { printf("mo duoc %s\n", paths[i]); fclose(f); }
    }
    return 0;
}
```

Trên Linux: `No such file or directory`, thư mục `.` mở `"r"` được nhưng đọc sẽ lỗi `Is a directory`, `/etc/shadow` cho `Permission denied` (nếu không phải root).

## Bài 2: `parse_int` và bộ kiểm thử

```c
// parse_int_test.c
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_int(const char *s, int *out) {
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (end == s || strpbrk(s, "0123456789") == NULL) return -1;   // rỗng / không có số
    if (*end != '\0') return -1;                   // ký tự thừa (kể cả khoảng trắng cuối)
    if (errno == ERANGE || v < INT_MIN || v > INT_MAX) return -1;
    *out = (int)v;
    return 0;
}

int main(void) {
    struct { const char *in; int ok; int val; } T[] = {
        { "0", 1, 0 }, { "42", 1, 42 }, { "-42", 1, -42 }, { "+7", 1, 7 },
        { "2147483647", 1, INT_MAX }, { "-2147483648", 1, INT_MIN },
        { "2147483648", 0, 0 }, { "-2147483649", 0, 0 },
        { "", 0, 0 }, { "abc", 0, 0 }, { "12abc", 0, 0 }, { "12 ", 0, 0 },
        { " 12", 1, 12 },                            // strtol bỏ qua khoảng trắng ĐẦU — ghi rõ hành vi này
        { "99999999999999999999", 0, 0 },
    };
    int failed = 0;
    for (size_t i = 0; i < sizeof T / sizeof T[0]; i++) {
        int v = 0;
        int ok = parse_int(T[i].in, &v) == 0;
        if (ok != T[i].ok || (ok && v != T[i].val)) { printf("FAIL: \"%s\"\n", T[i].in); failed++; }
    }
    printf("%d that bai\n", failed);
    return failed != 0;
}
```

Kiểm thử theo thuộc tính: với mọi `x` ngẫu nhiên, `snprintf(buf, "%d", x)` rồi `parse_int(buf)` phải trả lại đúng `x`.

## Bài 3: `goto cleanup` và `ErrorCode`

Mẫu đã có ở `process_file` (mục 13.5): mọi tài nguyên khởi tạo `NULL`, mọi lỗi `rc = ...; goto cleanup;`, phần `cleanup` giải phóng theo thứ tự ngược lại và trả `rc`. Kiểm chứng bằng ASan/LeakSanitizer với các kịch bản: file nguồn không có, file đích không tạo được (thư mục không ghi được), `malloc` thất bại (dùng fault injection, bài 5).

## Bài 4: `log.h` / `log.c` với `LOG_LEVEL` từ môi trường

Thêm vào `log.c` (mục 13.9) một hàm khởi tạo:

```c
// log_init.c
#include <stdlib.h>
#include <string.h>

typedef enum { LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR } LogLevel;
LogLevel g_log_level_from_env(void) {
    const char *e = getenv("LOG_LEVEL");
    if (!e) return LOG_INFO;
    if (strcmp(e, "DEBUG") == 0) return LOG_DEBUG;
    if (strcmp(e, "WARN")  == 0) return LOG_WARN;
    if (strcmp(e, "ERROR") == 0) return LOG_ERROR;
    return LOG_INFO;
}
```

Gọi `g_log_level = g_log_level_from_env();` ở đầu `main`. Để ghi ra file, mở `FILE *` bằng `"a"` và thay `stderr` trong `log_write` bằng biến toàn cục `g_log_out`.

## Bài 5: fault injection cho `Vec`

Ý tưởng (mục 13.10): thay `realloc` bằng bản có thể được lệnh thất bại ở lần gọi thứ `k`, rồi lặp `k = 0, 1, 2, …`:

```c
// vec_fault.c
#include <stdio.h>
#include <stdlib.h>

static int g_fail_at = -1;                                  // thất bại ở lần gọi thứ g_fail_at (-1: không)
static void *test_realloc(void *p, size_t n) {
    if (g_fail_at == 0) return NULL;
    if (g_fail_at > 0) g_fail_at--;
    return realloc(p, n);
}

typedef struct { int *data; size_t size, cap; } Vec;

static int vec_push(Vec *v, int x) {
    if (v->size == v->cap) {
        size_t nc = v->cap ? v->cap * 2 : 4;
        int *t = test_realloc(v->data, nc * sizeof *t);
        if (!t) return -1;                                  // v->data còn nguyên: không rò rỉ, không hỏng
        v->data = t; v->cap = nc;
    }
    v->data[v->size++] = x;
    return 0;
}

int main(void) {
    for (int k = 0; k < 6; k++) {                           // thử làm lỗi ở lần realloc thứ k
        Vec v = {0};
        g_fail_at = k;
        int failed = 0;
        for (int i = 0; i < 100; i++) if (vec_push(&v, i) != 0) { failed = 1; break; }
        printf("k=%d: %s, size=%zu\n", k, failed ? "lỗi được xử lý" : "không lỗi", v.size);
        free(v.data);                                        // dưới ASan: không được báo rò rỉ
    }
    return 0;
}
```

Mỗi lần lỗi, `vec_push` trả `-1`, `size` giữ nguyên tính nhất quán và không có rò rỉ — chạy với `-fsanitize=address` để xác nhận.

## Bài 6: xử lý `SIGINT`

```c
// sigint.c
#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static volatile sig_atomic_t g_count = 0;

static void on_sigint(int sig) {
    (void)sig;
    if (++g_count >= 2) _exit(130);            // lần hai: thoát ngay (chỉ dùng hàm an toàn với tín hiệu)
}

int main(void) {
    struct sigaction sa;
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    FILE *f = fopen("work.tmp", "w");
    while (g_count == 0) {
        if (f) fputs("tick\n", f);
        sleep(1);
    }
    printf("nhan Ctrl+C: dong file va thoat em\n");     // ngoài handler nên dùng printf được
    if (f) fclose(f);
    return 0;
}
```

Handler chỉ đặt cờ/đếm; việc dọn dẹp (đóng file, xả bộ đệm) làm trong luồng chính khi thấy cờ.

## Bài 7: macro `CHECK`

```c
// check_macro.c
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr, rc_var, code)                                                   \
    do {                                                                            \
        if (!(expr)) {                                                              \
            fprintf(stderr, "%s:%d: '%s' that bai: %s\n", __FILE__, __LINE__, #expr, strerror(errno)); \
            (rc_var) = (code);                                                      \
            goto cleanup;                                                           \
        }                                                                           \
    } while (0)

int main(void) {
    int rc = 0;
    FILE *f = NULL;
    CHECK((f = fopen("khong_co.txt", "r")) != NULL, rc, 1);
    fclose(f);
cleanup:
    return rc;
}
```

Macro `goto` cần nhãn `cleanup` tồn tại trong hàm gọi — hạn chế này là lý do một số người thích viết `if` tường minh.

## Bài 8: "try/catch" giả bằng `setjmp`/`longjmp`

```c
// trycatch.c
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_DEPTH 16
static jmp_buf g_stack[MAX_DEPTH];
static int g_depth = 0;

#define TRY   if (g_depth < MAX_DEPTH && setjmp(g_stack[g_depth++]) == 0) {
#define CATCH } else {
#define ENDTRY }
#define THROW(code) do { if (g_depth > 0) longjmp(g_stack[--g_depth], (code)); abort(); } while (0)

static void risky(int x) {
    if (x > 2) THROW(x);
}

int main(void) {
    TRY
        risky(1);
        risky(5);
        printf("khong toi day\n");
        g_depth--;                       // thoát khỏi TRY bình thường phải bỏ khung
    CATCH
        printf("bat duoc loi\n");
    ENDTRY
    return 0;
}
```

Các cách rò rỉ tài nguyên: mọi `malloc`/`fopen` ở các tầng hàm bị nhảy qua **không** được giải phóng; cách tránh: chỉ `THROW` khi mọi tài nguyên đã được quản lý theo mô hình arena/danh sách tự dọn, hoặc đăng ký tài nguyên vào "khung TRY" để `CATCH` tự dọn — tức là bạn đã tự cài đặt lại ngoại lệ với mọi rủi ro của nó, nên trong đa số dự án C, mã lỗi + `goto cleanup` vẫn là lựa chọn tốt hơn.
