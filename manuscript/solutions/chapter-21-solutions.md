# Chương 21 — Lời giải bài tập

## Bài 1: kiểm thử `Vec` với `--wrap=realloc`

```c
// vec_test.c
#include <stdio.h>
#include <stdlib.h>
#include "mini_test.h"

typedef struct { int *data; size_t size, cap; } Vec;

/* --- mã cần thử (thường nằm ở vec.c) --- */
static int g_fail_realloc_at = -1;
static void *my_realloc(void *p, size_t n) {           // bản giả: thất bại ở lần thứ k
    if (g_fail_realloc_at == 0) return NULL;
    if (g_fail_realloc_at > 0) g_fail_realloc_at--;
    return realloc(p, n);
}

static int vec_push(Vec *v, int x) {
    if (v->size == v->cap) {
        size_t nc = v->cap ? v->cap * 2 : 4;
        int *t = my_realloc(v->data, nc * sizeof *t);
        if (!t) return -1;
        v->data = t; v->cap = nc;
    }
    v->data[v->size++] = x;
    return 0;
}
static int vec_pop(Vec *v, int *out) { if (!v->size) return -1; *out = v->data[--v->size]; return 0; }

/* --- test --- */
static void test_push_many(void) {
    Vec v = {0};
    for (int i = 0; i < 1000; i++) ASSERT_EQ_INT(vec_push(&v, i), 0);
    ASSERT_EQ_INT((long long)v.size, 1000);
    ASSERT_TRUE(v.cap >= 1000);
    free(v.data);
}

static void test_pop_empty(void) {
    Vec v = {0};
    int x;
    ASSERT_EQ_INT(vec_pop(&v, &x), -1);
}

static void test_realloc_failure_is_clean(void) {
    for (int k = 0; k < 4; k++) {
        Vec v = {0};
        g_fail_realloc_at = k;
        int failed = 0;
        for (int i = 0; i < 100; i++) if (vec_push(&v, i) != 0) { failed = 1; break; }
        ASSERT_TRUE(failed);                          // đã thất bại đúng như giả lập
        ASSERT_TRUE(v.size <= v.cap);                 // trạng thái vẫn nhất quán
        free(v.data);                                 // ASan: không rò rỉ, không double free
    }
    g_fail_realloc_at = -1;
}

int main(void) {
    RUN_TEST(test_push_many);
    RUN_TEST(test_pop_empty);
    RUN_TEST(test_realloc_failure_is_clean);
    TEST_MAIN_END();
}
```

Với `-Wl,--wrap=realloc` bạn có thể thử **mã thật** mà không cần `my_realloc`: định nghĩa `__wrap_realloc` gọi `__real_realloc` (mục 21.5).

## Bài 2: thử `parse_int` và test theo thuộc tính

Dùng bộ 15 trường hợp ở lời giải chương 13 (bài 2) chuyển sang `ASSERT_*`. Thuộc tính vòng tròn:

```c
// roundtrip_test.c
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "mini_test.h"

static int parse_int(const char *s, int *out) {
    char *end;
    long v = strtol(s, &end, 10);
    if (end == s || *end != '\0' || v < INT_MIN || v > INT_MAX) return -1;
    *out = (int)v;
    return 0;
}

static void test_roundtrip(void) {
    srand(1234);                                                   // seed cố định: tái hiện được
    int samples[] = { 0, 1, -1, INT_MAX, INT_MIN };
    for (int iter = 0; iter < 2000; iter++) {
        int x = iter < 5 ? samples[iter] : (int)(((unsigned)rand() << 16) ^ (unsigned)rand());
        char buf[32];
        snprintf(buf, sizeof buf, "%d", x);
        int y = 0;
        ASSERT_EQ_INT(parse_int(buf, &y), 0);
        ASSERT_EQ_INT(y, x);
    }
}

int main(void) { RUN_TEST(test_roundtrip); TEST_MAIN_END(); }
```

## Bài 3: CTest nhiều module và golden file

Trong `CMakeLists.txt`, mỗi module một `add_executable` + `add_test`. Test đầu-cuối:

```cmake
add_test(NAME golden_case1
  COMMAND ${CMAKE_COMMAND} -DPROG=$<TARGET_FILE:app> -DIN=${CMAKE_SOURCE_DIR}/tests/case1.in
          -DEXPECTED=${CMAKE_SOURCE_DIR}/tests/case1.expected -P ${CMAKE_SOURCE_DIR}/tests/run_golden.cmake)
```

`run_golden.cmake` chạy `execute_process(COMMAND ${PROG} INPUT_FILE ${IN} OUTPUT_VARIABLE out)`, đọc `EXPECTED`, `if(NOT out STREQUAL expected) message(FATAL_ERROR ...)`. Hoặc đơn giản hơn: một script shell `run_golden.sh` chạy `diff`.

## Bài 4: coverage

Sau `gcov`, tìm các dòng `#####` trong hàm xử lý lỗi (ví dụ nhánh `malloc` trả `NULL`) và viết test chạy qua chúng (dùng `--wrap`/fault injection). Coverage tăng nghĩa là **mã đã được chạy**; **không** có nghĩa mã đúng — cần assert kiểm tra kết quả. Một test không có `ASSERT` vẫn làm coverage tăng.

## Bài 5: `ci.yml`

Kết hợp ba khối ở mục 21.10: ma trận `os × cc` (loại `macos + gcc`), một job `sanitizers` (`-DENABLE_SANITIZERS=ON`), một job `static-analysis` (`cppcheck --error-exitcode=1`). Để thấy CI đỏ: commit tạm một `use-after-free` (`free(p); *p = 1;`), quan sát job sanitizer thất bại với `heap-use-after-free`; sửa và job xanh. Nhớ `fail-fast: false` để thấy kết quả của mọi tổ hợp.

## Bài 6: fuzz trong CI

```yaml
  fuzz:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - run: sudo apt-get update && sudo apt-get install -y clang
      - run: clang -std=c11 -g -O1 -fsanitize=fuzzer,address,undefined fuzz/fuzz_parse.c src/parser.c -Iinclude -o fuzz_parse
      - run: mkdir -p corpus && ./fuzz_parse corpus -max_total_time=60 -artifact_prefix=./crash-
      - if: failure()
        uses: actions/upload-artifact@v4
        with: { name: fuzz-crashes, path: crash-* }
```

Chép các đầu vào gây lỗi đã sửa vào `tests/regression/` để mỗi lần chạy CI chạy lại chúng.

## Bài 7: refactor chương trình khó thử

Ví dụ: hàm `report()` (đọc `scores.txt`, in trung bình) thành: `stats_compute(const int *, size_t, Stats *)` (lõi thuần, test bằng mảng trong bộ nhớ) + `read_ints(FILE *, int **, size_t *)` (I/O, test bằng `fmemopen`) + `main` mỏng. Những gì phải đổi: bỏ `printf` trong lõi (trả kết quả), bỏ file cố định (nhận `FILE *`), xử lý `n == 0` (trước đây chia cho 0), và trả mã lỗi thay vì `exit`.

## Bài 8: đồng hồ giả cho TTL cache

```c
// ttl_test.c
#include <stdio.h>
#include <time.h>
#include "mini_test.h"

typedef struct { time_t (*now)(void); } Clock;
typedef struct { int value; time_t expires; int valid; } Entry;

static void entry_set(Entry *e, int v, time_t ttl, const Clock *c) { e->value = v; e->expires = c->now() + ttl; e->valid = 1; }
static int  entry_get(const Entry *e, const Clock *c, int *out) {
    if (!e->valid || c->now() >= e->expires) return -1;         // hết hạn khi now >= expires
    *out = e->value;
    return 0;
}

static time_t g_now;
static time_t fake_now(void) { return g_now; }

static void test_ttl(void) {
    Clock c = { fake_now };
    Entry e = {0};
    int v;
    g_now = 1000;
    entry_set(&e, 42, 60, &c);
    g_now = 1059; ASSERT_EQ_INT(entry_get(&e, &c, &v), 0);      // còn hạn
    g_now = 1060; ASSERT_EQ_INT(entry_get(&e, &c, &v), -1);     // đúng lúc hết hạn
    g_now = 5000; ASSERT_EQ_INT(entry_get(&e, &c, &v), -1);
}

int main(void) { RUN_TEST(test_ttl); TEST_MAIN_END(); }
```

Không có `sleep` nên test chạy tức thì và tất định; giờ thật chỉ xuất hiện ở lớp ứng dụng (`Clock real = { time_wrapper }`).

## Bài 9: kiểm tra hiệu năng trong CI

Runner dùng chung nhiễu 10–30%, nên so thời gian tuyệt đối dễ báo sai. Cách tốt: đo bằng **`valgrind --tool=callgrind`** (đếm lệnh, ổn định ±0,1%) rồi so số lệnh với giá trị lưu trong repo: `callgrind_annotate callgrind.out | grep PROGRAM TOTALS`; thất bại nếu tăng > 20%. Nếu bắt buộc dùng thời gian: chạy nhiều lần, lấy **min**, đặt ngưỡng rộng, và chạy trên runner tự host cố định CPU (`taskset`, tắt turbo).
