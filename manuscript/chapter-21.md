# Chương 21 — Testing & CI cho C

## Mục tiêu chương

- Hiểu vì sao kiểm thử tự động đặc biệt quan trọng với C và **kim tự tháp kiểm thử**.
- Viết mã **dễ kiểm thử** (testable): tách logic khỏi I/O, dùng phụ thuộc có thể thay thế.
- Viết **unit test** bằng khung tự làm và bằng framework: **Unity**, **Check**, **Criterion** (so sánh).
- Dùng **mock/stub/fake** để cô lập mã cần thử: con trỏ hàm, *link seam*, fault injection.
- Tích hợp với build: **`make test`** và **CTest (CMake)**.
- Đo **độ phủ mã (coverage)** bằng `gcov`/`lcov`; hiểu chỉ số này nói được gì và không nói được gì.
- Chạy **sanitizer**, **fuzz**, và phân tích tĩnh trong kiểm thử.
- Thiết lập **CI** với **GitHub Actions**: build ma trận (Linux/macOS/Windows, gcc/clang), chạy test, sanitizer, coverage, lưu artifact.

## 21.1. Vì sao phải kiểm thử tự động?

Trong C, một thay đổi nhỏ có thể phá hỏng chỗ rất xa (con trỏ, bộ nhớ, macro). Không có kiểm thử, bạn chỉ phát hiện lỗi khi người dùng gặp. Kiểm thử tự động mang lại:

1. **Phát hiện hồi quy (regression):** mỗi lần sửa mã, chạy lại toàn bộ bài thử trong vài giây.
2. **Tự tin refactor/tối ưu** (chương 17): kết quả vẫn đúng thì yên tâm.
3. **Tài liệu sống:** bài thử cho thấy cách dùng hàm và các trường hợp biên.
4. **Thiết kế tốt hơn:** mã khó kiểm thử thường là mã ghép nối chặt, khó tái sử dụng.
5. **Bắt lỗi bộ nhớ** khi kết hợp sanitizer (chương 9, 18).

### Kim tự tháp kiểm thử

```text
                /\
               /  \      Kiểm thử đầu-cuối (E2E): chạy cả chương trình
              /----\     ít, chậm, mong manh
             /      \    Kiểm thử tích hợp: nhiều module cùng nhau
            /--------\   vừa phải
           /          \  Kiểm thử đơn vị (unit): từng hàm/module
          /____________\ nhiều, nhanh (mili-giây), ổn định
```

Nguyên tắc: **nhiều unit test nhanh ở đáy, ít test chậm ở đỉnh.** Một bộ unit test tốt chạy toàn bộ trong vài giây.

### Một unit test tốt

- **Nhanh** (mili-giây), **độc lập** (không phụ thuộc thứ tự chạy, không phụ thuộc test khác), **lặp lại được** (cùng kết quả mỗi lần, không phụ thuộc giờ/mạng/ngẫu nhiên).
- **Tự kiểm tra:** đạt/không đạt do máy quyết định, không cần con người đọc đầu ra.
- **Một ý một test**, tên nói rõ **cái gì, trong điều kiện nào, kết quả mong đợi**: `test_parse_int_rejects_trailing_garbage`.
- Mẫu **Arrange – Act – Assert**: chuẩn bị dữ liệu → gọi hàm → so sánh kết quả.

## 21.2. Viết mã dễ kiểm thử

Kiểm thử khó khi hàm **làm quá nhiều việc cùng lúc**: đọc file, tính toán, in ra, đọc giờ hệ thống.

### Ví dụ khó kiểm thử

```c
// KHÓ THỬ: trộn I/O, tính toán và trạng thái toàn cục
void report(void) {
    FILE *f = fopen("scores.txt", "r");          // phụ thuộc file thật
    int sum = 0, n = 0, x;
    while (fscanf(f, "%d", &x) == 1) { sum += x; n++; }
    fclose(f);
    printf("Trung binh: %.2f\n", (double)sum / n);   // kết quả đi thẳng ra màn hình, và chia cho 0 nếu n = 0
}
```

### Tách phần lõi thuần khiết

```c
// stats.h — logic THUẦN: nhận dữ liệu, trả kết quả, không I/O, không toàn cục
#include <stddef.h>
typedef struct { size_t count; double mean, min, max; } Stats;

// Trả 0 nếu tính được, -1 nếu n == 0 hoặc data == NULL.
int stats_compute(const int *data, size_t n, Stats *out);
```

```c
// stats.c
#include "stats.h"
int stats_compute(const int *data, size_t n, Stats *out) {
    if (!data || n == 0 || !out) return -1;
    long sum = 0;
    int mn = data[0], mx = data[0];
    for (size_t i = 0; i < n; i++) {
        sum += data[i];
        if (data[i] < mn) mn = data[i];
        if (data[i] > mx) mx = data[i];
    }
    out->count = n;
    out->mean  = (double)sum / (double)n;
    out->min   = mn;
    out->max   = mx;
    return 0;
}
```

`main` (mỏng) lo I/O và gọi lõi. Lõi được test dễ dàng bằng mảng nhỏ trong bộ nhớ.

### Nguyên tắc thiết kế cho khả năng kiểm thử

1. **Tách logic khỏi I/O:** hàm thuần dễ thử nhất. Đưa I/O ra rìa (imperative shell, functional core).
2. **Tránh trạng thái toàn cục** (hoặc cho phép reset). Truyền ngữ cảnh qua tham số/struct.
3. **Trả kết quả và mã lỗi** thay vì in trực tiếp — test kiểm tra được giá trị.
4. **Nhận phụ thuộc qua tham số** (con trỏ hàm, struct chứa các hàm) để thay bằng bản giả khi thử.
5. **Đừng gọi `exit()` trong thư viện** (làm sập cả bộ test).
6. **Header rõ ràng, hàm nhỏ**, dùng `static` cho nội bộ nhưng **test qua giao diện công khai**; nếu cần thử hàm `static`, cân nhắc `#include "file.c"` trong file test, hoặc nâng nó lên thành công khai có tiền tố nội bộ.
7. **Tất định (deterministic):** thời gian, số ngẫu nhiên, ID đều nhận từ bên ngoài (ví dụ truyền `seed`, truyền hàm `now()`).

## 21.3. Khung kiểm thử tối giản tự làm

Bạn đã thấy `tiny_test.h` ở chương 14. Bản mở rộng có **tên test, đếm, và tự đăng ký**:

```c
// mini_test.h
#ifndef MINI_TEST_H
#define MINI_TEST_H
#include <math.h>
#include <stdio.h>
#include <string.h>

static int mt_checks = 0, mt_failed = 0, mt_current_failed = 0;

#define ASSERT_TRUE(cond) do {                                                   \
        mt_checks++;                                                             \
        if (!(cond)) {                                                           \
            mt_failed++; mt_current_failed = 1;                                  \
            fprintf(stderr, "  %s:%d: ASSERT_TRUE(%s) that bai\n", __FILE__, __LINE__, #cond); \
        }                                                                        \
    } while (0)

#define ASSERT_EQ_INT(actual, expected) do {                                     \
        long long a_ = (actual), e_ = (expected);                                \
        mt_checks++;                                                             \
        if (a_ != e_) {                                                          \
            mt_failed++; mt_current_failed = 1;                                  \
            fprintf(stderr, "  %s:%d: %s = %lld, mong doi %lld\n", __FILE__, __LINE__, #actual, a_, e_); \
        }                                                                        \
    } while (0)

#define ASSERT_EQ_STR(actual, expected) do {                                     \
        const char *a_ = (actual), *e_ = (expected);                             \
        mt_checks++;                                                             \
        if (a_ == NULL || e_ == NULL || strcmp(a_, e_) != 0) {                   \
            mt_failed++; mt_current_failed = 1;                                  \
            fprintf(stderr, "  %s:%d: %s = \"%s\", mong doi \"%s\"\n", __FILE__, __LINE__, #actual, a_ ? a_ : "(null)", e_ ? e_ : "(null)"); \
        }                                                                        \
    } while (0)

#define ASSERT_NEAR(actual, expected, eps) do {                                  \
        double a_ = (actual), e_ = (expected);                                   \
        mt_checks++;                                                             \
        if (fabs(a_ - e_) > (eps)) {                                             \
            mt_failed++; mt_current_failed = 1;                                  \
            fprintf(stderr, "  %s:%d: %s = %g, mong doi %g (+-%g)\n", __FILE__, __LINE__, #actual, a_, e_, (double)(eps)); \
        }                                                                        \
    } while (0)

// Chạy một test và in kết quả
#define RUN_TEST(fn) do {                                                        \
        mt_current_failed = 0;                                                   \
        fn();                                                                    \
        printf("[%s] %s\n", mt_current_failed ? "FAIL" : " OK ", #fn);           \
    } while (0)

#define TEST_MAIN_END() do {                                                     \
        printf("\n%d kiem tra, %d that bai\n", mt_checks, mt_failed);            \
        return mt_failed ? 1 : 0;                                                \
    } while (0)

#endif
```

```c
// test_stats.c
#include "mini_test.h"
#include "stats.h"

static void test_compute_basic(void) {
    int d[] = {1, 2, 3, 4, 5};
    Stats s;
    ASSERT_EQ_INT(stats_compute(d, 5, &s), 0);
    ASSERT_NEAR(s.mean, 3.0, 1e-9);
    ASSERT_EQ_INT((long long)s.min, 1);
    ASSERT_EQ_INT((long long)s.max, 5);
}

static void test_compute_single(void) {
    int d[] = {42};
    Stats s;
    ASSERT_EQ_INT(stats_compute(d, 1, &s), 0);
    ASSERT_NEAR(s.mean, 42.0, 1e-9);
}

static void test_compute_negative(void) {
    int d[] = {-5, -1, -10};
    Stats s;
    ASSERT_EQ_INT(stats_compute(d, 3, &s), 0);
    ASSERT_EQ_INT((long long)s.min, -10);
    ASSERT_EQ_INT((long long)s.max, -1);
}

static void test_compute_rejects_empty(void) {
    Stats s;
    ASSERT_EQ_INT(stats_compute(NULL, 0, &s), -1);
    int d[] = {1};
    ASSERT_EQ_INT(stats_compute(d, 0, &s), -1);    // n == 0
}

int main(void) {
    RUN_TEST(test_compute_basic);
    RUN_TEST(test_compute_single);
    RUN_TEST(test_compute_negative);
    RUN_TEST(test_compute_rejects_empty);
    TEST_MAIN_END();
}
```

Biên dịch và chạy (**mã thoát khác 0 khi có test lỗi** — đây là điều CI/`make test` dùng để nhận biết):

```bash
gcc -std=c11 -Wall -Wextra -g -fsanitize=address,undefined test_stats.c stats.c -o test_stats -lm
./test_stats; echo "exit=$?"
```

Kết quả mong đợi:

```text
[ OK ] test_compute_basic
[ OK ] test_compute_single
[ OK ] test_compute_negative
[ OK ] test_compute_rejects_empty

11 kiem tra, 0 that bai
```

Một khung tự làm rất gọn cho dự án nhỏ, nhưng thiếu: `setUp`/`tearDown`, chạy độc lập từng test, báo cáo XML cho CI, cô lập crash (một test `segfault` làm sập cả chương trình). Framework thật giải quyết những thứ đó.

## 21.4. Các framework kiểm thử cho C

| Framework | Đặc điểm | Phù hợp |
|---|---|---|
| **Unity** (ThrowTheSwitch) | Rất nhẹ (1 file `.c` + 2 `.h`), di động, chạy được trên vi điều khiển; nhiều macro assert; kèm CMock/Ceedling | Dự án nhúng, muốn tối giản |
| **Check** | Chạy mỗi test trong **tiến trình con** (`fork`) nên crash không làm hỏng bộ test; hỗ trợ fixture, timeout | Linux/macOS, cần cô lập |
| **Criterion** | Hiện đại: tự đăng ký test, tham số hóa, xuất TAP/JSON/XML, nhiều output đẹp | Dự án Linux/macOS mới |
| **CMocka** | Có sẵn **mock** (dùng linker `--wrap`), di động | Cần mock hàm C |
| **cmocka/µnit/greatest** | Nhẹ, một header | Nhỏ gọn |
| **GoogleTest (C++)** | Rất mạnh; có thể test mã C nếu chấp nhận biên dịch C++ | Dự án hỗn hợp C/C++ |

### Ví dụ với Unity

```c
// test_stats_unity.c
#include "unity.h"
#include "stats.h"

void setUp(void)    { /* chạy TRƯỚC mỗi test */ }
void tearDown(void) { /* chạy SAU mỗi test */ }

void test_mean_of_five(void) {
    int d[] = {1, 2, 3, 4, 5};
    Stats s;
    TEST_ASSERT_EQUAL_INT(0, stats_compute(d, 5, &s));
    TEST_ASSERT_EQUAL_DOUBLE(3.0, s.mean);
    TEST_ASSERT_EQUAL_INT(1, (int)s.min);
}

void test_empty_input_fails(void) {
    Stats s;
    TEST_ASSERT_EQUAL_INT(-1, stats_compute(NULL, 0, &s));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_mean_of_five);
    RUN_TEST(test_empty_input_fails);
    return UNITY_END();
}
```

Unity in kết quả kiểu `test_stats_unity.c:12:test_mean_of_five:PASS` và tóm tắt `2 Tests 0 Failures 0 Ignored`. Có các assert: `TEST_ASSERT_EQUAL_STRING`, `TEST_ASSERT_EQUAL_MEMORY`, `TEST_ASSERT_NULL`, `TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)`, `TEST_ASSERT_EQUAL_INT_ARRAY`...

Lấy Unity: sao chép `unity.c`, `unity.h`, `unity_internals.h` vào `third_party/unity/`, hoặc dùng CMake `FetchContent`.

### Ví dụ với Check

```c
#include <check.h>
#include "stats.h"

START_TEST(test_mean) {
    int d[] = {2, 4, 6};
    Stats s;
    ck_assert_int_eq(stats_compute(d, 3, &s), 0);
    ck_assert_double_eq_tol(s.mean, 4.0, 1e-9);
}
END_TEST

static Suite *stats_suite(void) {
    Suite *s = suite_create("stats");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, test_mean);
    suite_add_tcase(s, tc);
    return s;
}

int main(void) {
    SRunner *sr = srunner_create(stats_suite());
    srunner_run_all(sr, CK_NORMAL);
    int failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return failed ? 1 : 0;
}
```

Biên dịch: `gcc test.c stats.c $(pkg-config --cflags --libs check) -o test`.

### Ví dụ với Criterion

```c
#include <criterion/criterion.h>
#include "stats.h"

Test(stats, mean_of_three) {
    int d[] = {2, 4, 6};
    Stats s;
    cr_assert_eq(stats_compute(d, 3, &s), 0);
    cr_assert_float_eq(s.mean, 4.0, 1e-9);
}
```

Không cần `main` — Criterion tự sinh và tự phát hiện test. Liên kết: `-lcriterion`.

### Chọn cái nào?

- **Dự án nhỏ/học tập:** khung tự làm hoặc Unity (ít phụ thuộc).
- **Cần cô lập crash và fixture:** Check hoặc Criterion (Linux/macOS).
- **Cần mock hàm C:** CMocka hoặc CMock (đi cùng Unity).
- **Ưu tiên tiện lợi và đầu ra đẹp:** Criterion.

Điều quan trọng hơn framework: **có bộ test chạy tự động trong CI**.

## 21.5. Mock, stub và fake — cô lập phụ thuộc

Khi hàm cần thử phụ thuộc vào thứ **chậm, không tất định, hoặc khó tạo lỗi** (đĩa, mạng, giờ hệ thống, `malloc` lỗi), ta thay nó bằng một bản giả trong lúc thử.

| Thuật ngữ | Ý nghĩa |
|---|---|
| **Stub** | Trả về giá trị cố định được lập trình sẵn |
| **Fake** | Cài đặt đơn giản có hành vi thật (ví dụ hệ thống file trong bộ nhớ) |
| **Mock** | Stub có thể **kiểm tra xem nó đã được gọi thế nào** (đúng đối số, số lần) |

### Kỹ thuật 1: Nhận phụ thuộc qua con trỏ hàm

```c
/* ví dụ: tiêm phụ thuộc (cache.h) */
typedef struct {
    time_t (*now)(void);                       // nguồn thời gian có thể thay
} Clock;

int session_is_expired(const Session *s, const Clock *clk);
```

```c
/* ví dụ: session.c */
int session_is_expired(const Session *s, const Clock *clk) {
    return clk->now() > s->expires_at;         // dùng "đồng hồ" được tiêm vào
}
```

```c
/* ví dụ: test_session.c */
static time_t fake_now_value;
static time_t fake_now(void) { return fake_now_value; }

static void test_session_expires(void) {
    Session s = { .expires_at = 1000 };
    Clock clk = { .now = fake_now };

    fake_now_value = 999;   ASSERT_EQ_INT(session_is_expired(&s, &clk), 0);
    fake_now_value = 1001;  ASSERT_EQ_INT(session_is_expired(&s, &clk), 1);
}
```

Trong sản phẩm, truyền `Clock real = { .now = time_wrapper };`. Bài thử không phải chờ hay phụ thuộc giờ thật.

### Kỹ thuật 2: Đường nối liên kết (link seam) — thay hàm lúc liên kết

Với thư viện bạn không sửa được, hoặc hàm gọi trực tiếp (không qua con trỏ), có thể **định nghĩa lại hàm trong file test** và liên kết file test thay vì bản thật:

```c
// production: net.c gọi send_data(...) — cài đặt thật trong net_real.c
// test: định nghĩa bản giả
static char last_sent[256]; static int send_calls;
int send_data(const char *buf, size_t n) {         // bản giả trong test_client.c
    send_calls++;
    snprintf(last_sent, sizeof last_sent, "%.*s", (int)n, buf);
    return (int)n;
}
```

Biên dịch test **không** kèm `net_real.c`: `gcc test_client.c client.c -o test_client`. Đây là cách CMock/Ceedling tự sinh.

Với gcc/ld: `-Wl,--wrap=malloc` cho phép chặn lời gọi `malloc`: mọi lời gọi `malloc` được chuyển thành `__wrap_malloc` (bạn định nghĩa), và `__real_malloc` là bản gốc.

```c
extern void *__real_malloc(size_t n);
static int g_fail_malloc;
void *__wrap_malloc(size_t n) {
    if (g_fail_malloc) return NULL;                // giả lập hết bộ nhớ
    return __real_malloc(n);
}
```

```bash
gcc test_vec.c vec.c -Wl,--wrap=malloc -o test_vec
```

Nhờ đó **kiểm thử đường lỗi** (`malloc` trả NULL) một cách tất định — bổ sung cho fault injection ở chương 13.

### Kỹ thuật 3: Fake trong bộ nhớ

Thay vì thử với file thật, cung cấp `FILE *` từ bộ nhớ: `fmemopen(buf, size, "r")` (POSIX) hoặc `tmpfile()`; hoặc thiết kế hàm nhận `const char *` thay vì `FILE *` để không cần file:

```c
static void test_parse_csv_line_from_string(void) {
    FILE *f = fmemopen((void *)"an,9\nbinh,8\n", 12, "r");
    /* ... gọi hàm đọc từ f ... */
    fclose(f);
}
```

## 21.6. Các loại test cần có

### 1. Trường hợp thường, biên và lỗi

Với mỗi hàm, hãy nghĩ tới:

- **Đầu vào điển hình.**
- **Biên:** 0, 1, phần tử đầu/cuối, `INT_MAX`, `INT_MIN`, chuỗi rỗng, chuỗi dài đúng bằng bộ đệm, chuỗi dài bộ đệm + 1.
- **Đầu vào không hợp lệ:** `NULL`, số âm, ký tự lạ.
- **Trạng thái lỗi:** hết bộ nhớ, file không có, đầu vào bị cắt cụt.

### 2. Test hồi quy

Mỗi khi gặp bug: **viết test làm bug tái hiện (fail)** trước, sửa mã, test chuyển thành pass, giữ test mãi mãi.

### 3. Test dựa trên thuộc tính (property-based) — thủ công

Thay vì chọn ví dụ, kiểm tra **tính chất luôn đúng** trên nhiều đầu vào ngẫu nhiên có seed cố định:

```c
static void test_sort_property(void) {
    srand(12345);                                       // seed cố định -> lặp lại được
    for (int iter = 0; iter < 1000; iter++) {
        int n = rand() % 100;
        int a[100], b[100];
        for (int i = 0; i < n; i++) a[i] = b[i] = rand() % 50 - 25;

        my_sort(a, (size_t)n);
        qsort(b, (size_t)n, sizeof b[0], cmp_int);      // tham chiếu ("oracle")
        ASSERT_TRUE(memcmp(a, b, (size_t)n * sizeof a[0]) == 0);
    }
}
```

So với một **bản tham chiếu đúng** (oracle) hoặc kiểm tra tính chất (đầu ra đã sắp xếp, là hoán vị của đầu vào). In `seed` và `iter` khi lỗi để tái hiện.

### 3b. Test vi sai (differential)

So kết quả của hai cài đặt (bản đơn giản và bản tối ưu, chương 17; evaluator và VM, chương 19) trên cùng đầu vào — rất hiệu quả bắt lỗi.

### 4. Test dữ liệu mẫu ("golden file")

Chạy chương trình với input cố định và so đầu ra với file mẫu đã duyệt (`expected.txt`): tiện cho trình biên dịch, bộ định dạng.

```bash
./calc < tests/case1.in > /tmp/out.txt && diff -u tests/case1.expected /tmp/out.txt
```

### 5. Test tích hợp/đầu-cuối

Chạy chương trình thật, kiểm tra mã thoát và đầu ra; với server: khởi chạy, gửi yêu cầu bằng `curl`, so phản hồi:

```bash
#!/usr/bin/env bash
set -euo pipefail
./server 18080 2 ./www & PID=$!
trap 'kill $PID 2>/dev/null || true' EXIT
sleep 0.5
code=$(curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:18080/)
[ "$code" = "200" ] || { echo "GET / -> $code (mong doi 200)"; exit 1; }
code=$(curl -s -o /dev/null -w '%{http_code}' --path-as-is http://127.0.0.1:18080/../server.c)
[ "$code" = "403" ] || { echo "traversal -> $code (mong doi 403)"; exit 1; }
echo "OK"
```

## 21.7. Tích hợp vào build: `make test` và CTest

### Makefile

```make
CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -g -O1 -Iinclude
SAN     ?= -fsanitize=address,undefined -fno-omit-frame-pointer

TEST_SRCS := $(wildcard tests/test_*.c)
TEST_BINS := $(patsubst tests/%.c,build/%,$(TEST_SRCS))
LIB_SRCS  := $(filter-out src/main.c,$(wildcard src/*.c))

.PHONY: test clean
test: $(TEST_BINS)
	@fail=0; for t in $(TEST_BINS); do \
	    echo "== $$t"; ./$$t || fail=1; \
	done; \
	exit $$fail

build/test_%: tests/test_%.c $(LIB_SRCS) | build
	$(CC) $(CFLAGS) $(SAN) $^ -o $@ -lm

build:
	mkdir -p build

clean:
	rm -rf build
```

`make test` biên dịch từng `tests/test_*.c` cùng thư viện (không kèm `main.c`), bật sanitizer, chạy tất cả và **trả mã khác 0 nếu bất kỳ test nào lỗi**.

### CMake + CTest

```cmake
cmake_minimum_required(VERSION 3.16)
project(myproj C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(ENABLE_SANITIZERS "Bat ASan/UBSan" ON)
option(ENABLE_COVERAGE   "Bat gcov"        OFF)

add_library(core src/stats.c)
target_include_directories(core PUBLIC include)
target_compile_options(core PRIVATE -Wall -Wextra -Wpedantic)

if(ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()
if(ENABLE_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

add_executable(app src/main.c)
target_link_libraries(app PRIVATE core m)

enable_testing()
foreach(t test_stats test_parser)
    add_executable(${t} tests/${t}.c)
    target_link_libraries(${t} PRIVATE core m)
    add_test(NAME ${t} COMMAND ${t})
endforeach()

# test đầu-cuối bằng script
add_test(NAME e2e COMMAND ${CMAKE_SOURCE_DIR}/tests/e2e.sh $<TARGET_FILE:app>)
```

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure        # chạy tất cả test; in đầu ra của test lỗi
ctest --test-dir build -R stats -V                # chạy test tên khớp "stats", chi tiết
```

CTest thu thập kết quả, hỗ trợ chạy song song (`-j4`), thời gian chờ (`set_tests_properties(... TIMEOUT 10)`), và xuất báo cáo JUnit cho CI.

## 21.8. Độ phủ mã (coverage)

**Coverage** cho biết **dòng/nhánh mã nào đã được test chạy tới**. Công cụ của gcc: `gcov`; báo cáo HTML: `lcov` + `genhtml`, hoặc `gcovr`.

```bash
gcc -std=c11 --coverage -O0 -g test_stats.c stats.c -o test_stats -lm
./test_stats                                       # sinh file .gcda
gcov stats.c                                       # sinh stats.c.gcov: từng dòng kèm số lần chạy
```

Kết quả `stats.c.gcov` (rút gọn):

```text
        5:    5:int stats_compute(const int *data, size_t n, Stats *out) {
        5:    6:    if (!data || n == 0 || !out) return -1;
        3:    7:    long sum = 0;
    #####:   19:    /* dòng chưa từng chạy */
```

`#####` là dòng **chưa được test chạy tới**. Báo cáo HTML:

```bash
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' 'tests/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
# hoặc gọn hơn:
gcovr -r . --html-details -o coverage.html --exclude 'tests/'
```

### Các loại coverage

| Loại | Đo | Ghi chú |
|---|---|---|
| **Dòng (line)** | Dòng nào được chạy | Phổ biến nhất |
| **Nhánh (branch)** | Mỗi `if` đã thử cả đúng và sai chưa | `gcov -b`; giá trị hơn dòng |
| **Hàm (function)** | Hàm nào được gọi | Thô |
| **MC/DC** | Từng điều kiện con trong biểu thức | Yêu cầu trong hàng không/ô tô |

### Coverage nói được gì — và không nói được gì

- Mã **chưa được chạy** chắc chắn **chưa được kiểm thử** → phát hiện chỗ thiếu test. Đây là công dụng chính.
- Nhưng **coverage cao ≠ test tốt.** Test có thể chạy qua mã mà không kiểm tra gì (không assert). 100% coverage vẫn có thể bỏ sót lỗi.
- Đừng biến nó thành mục tiêu tự thân (ép 100%). Mục tiêu hợp lý: **80–90% dòng cho mã lõi**, kèm test biên và test lỗi; quan tâm **nhánh chưa phủ** ở đường xử lý lỗi.
- Biên dịch bản coverage với `-O0` để số liệu tương ứng mã nguồn.

## 21.9. Sanitizer, phân tích tĩnh và fuzz trong kiểm thử

Kết hợp để bộ test **đồng thời bắt lỗi bộ nhớ** (chương 9 và 18):

| Công cụ | Khi nào chạy | Cách |
|---|---|---|
| **ASan + UBSan** | Mọi lần chạy test | `-fsanitize=address,undefined` |
| **TSan** | Với mã đa luồng (bản build riêng) | `-fsanitize=thread` |
| **MSan** (clang) | Bản build riêng | `-fsanitize=memory` |
| **Valgrind** | Định kỳ (chậm) | `valgrind --error-exitcode=1 --leak-check=full ./test` |
| **cppcheck / clang-tidy** | Mỗi commit | `cppcheck --error-exitcode=1 ...` |
| **`gcc -fanalyzer`** | Mỗi commit | thêm cờ biên dịch |
| **Fuzz** | Chạy định kỳ / khi đổi parser | libFuzzer/AFL++ (chương 18) |
| **`-Werror`** | CI | biến cảnh báo thành lỗi |

Đặt **`-Werror`** trong CI (nhưng không nhất thiết trên máy nhà phát triển) để không cho cảnh báo tích tụ.

Mã thoát đặc biệt: ASan mặc định thoát mã khác 0 khi phát hiện lỗi, nên CI tự thấy đỏ. Với Valgrind dùng `--error-exitcode=1`.

## 21.10. Tích hợp liên tục (CI) với GitHub Actions

**CI (Continuous Integration):** mỗi lần đẩy mã (`push`) hoặc mở pull request, một máy chủ **tự động** build và chạy kiểm thử. Lỗi được phát hiện ngay, trước khi hòa nhập vào nhánh chính.

**GitHub Actions** định nghĩa CI bằng file YAML trong `.github/workflows/`.

### Khái niệm

- **Workflow:** một file YAML mô tả quy trình.
- **Event (`on`):** điều gì kích hoạt (`push`, `pull_request`, `schedule`, `workflow_dispatch`).
- **Job:** nhóm bước chạy trên một máy (runner); các job chạy song song mặc định.
- **Step:** một lệnh shell (`run:`) hoặc một action dùng lại (`uses:`).
- **Runner:** máy ảo (`ubuntu-latest`, `macos-latest`, `windows-latest`).
- **Matrix:** chạy cùng job với nhiều tổ hợp tham số (hệ điều hành × compiler).

### Workflow cơ bản: build + test

```yaml
# .github/workflows/ci.yml
name: CI

on:
  push:
    branches: [main]
  pull_request:

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Cai cong cu
        run: sudo apt-get update && sudo apt-get install -y build-essential cmake valgrind cppcheck

      - name: Cau hinh
        run: cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

      - name: Build
        run: cmake --build build -j

      - name: Test
        run: ctest --test-dir build --output-on-failure
```

Đẩy file này lên nhánh `main` rồi xem tab **Actions** trên trang GitHub của kho mã. Dấu ✓ xanh hoặc ✗ đỏ hiện cạnh mỗi commit và pull request.

### Ma trận nhiều nền tảng và compiler

```yaml
jobs:
  build-test:
    name: ${{ matrix.os }} / ${{ matrix.cc }}
    runs-on: ${{ matrix.os }}
    strategy:
      fail-fast: false                     # một tổ hợp lỗi không hủy các tổ hợp khác
      matrix:
        os: [ubuntu-latest, macos-latest]
        cc: [gcc, clang]
        exclude:
          - os: macos-latest
            cc: gcc                         # trên macOS "gcc" là alias của clang
    env:
      CC: ${{ matrix.cc }}
    steps:
      - uses: actions/checkout@v4
      - run: cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=$CC
      - run: cmake --build build -j
      - run: ctest --test-dir build --output-on-failure
```

Thêm Windows: `windows-latest` (dùng MSVC mặc định; hoặc cài MinGW/MSYS2 qua `msys2/setup-msys2`). Chạy nhiều tổ hợp giúp phát hiện mã **không di động** (kích thước kiểu, `long` trên Windows, hàm POSIX không có trên MSVC...).

### Job sanitizer, phân tích tĩnh, coverage

```yaml
  sanitizers:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: ASan + UBSan
        run: |
          cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
          cmake --build build-asan -j
          ctest --test-dir build-asan --output-on-failure
        env:
          ASAN_OPTIONS: detect_leaks=1:abort_on_error=1
          UBSAN_OPTIONS: print_stacktrace=1:halt_on_error=1

  static-analysis:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - run: sudo apt-get update && sudo apt-get install -y cppcheck clang-tidy
      - run: cppcheck --enable=warning,performance,portability --error-exitcode=1 --inline-suppr src/

  coverage:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - run: sudo apt-get update && sudo apt-get install -y lcov cmake
      - run: |
          cmake -S . -B build-cov -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON -DENABLE_SANITIZERS=OFF
          cmake --build build-cov -j
          ctest --test-dir build-cov
          lcov --capture --directory build-cov --output-file coverage.info
          lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.info
          lcov --summary coverage.info
      - uses: actions/upload-artifact@v4
        with:
          name: coverage-report
          path: coverage.info
```

### Lưu artifact và nhật ký khi lỗi

```yaml
      - name: Tai len ket qua khi that bai
        if: failure()
        uses: actions/upload-artifact@v4
        with:
          name: test-logs
          path: build/Testing/Temporary/LastTest.log
```

### Bộ nhớ đệm để build nhanh hơn

```yaml
      - uses: actions/cache@v4
        with:
          path: ~/.cache/ccache
          key: ccache-${{ runner.os }}-${{ github.sha }}
          restore-keys: ccache-${{ runner.os }}-
```

(Kết hợp với `ccache` để không biên dịch lại file không đổi.)

### Bảo vệ nhánh chính

Trong **Settings → Branches → Branch protection rules** của kho mã: yêu cầu các job CI phải **xanh** trước khi merge pull request. Khi đó không thể đưa mã làm hỏng test vào `main`.

### Huy hiệu (badge) trạng thái

Thêm vào `README.md` để hiện trạng thái build:

```markdown
![CI](https://github.com/<user>/<repo>/actions/workflows/ci.yml/badge.svg)
```

### Bảo mật của CI

- Dùng `permissions: contents: read` ở mức workflow để **hạn chế quyền** của token.
- **Ghim phiên bản action** (`@v4`, tốt hơn là SHA commit) và cẩn trọng với action bên thứ ba.
- **Không in bí mật** ra log; lưu bí mật trong **Settings → Secrets**, không đặt trong mã. Không chạy mã từ pull request của người lạ với quyền truy cập bí mật (`pull_request_target` cần rất cẩn thận).

## 21.11. Quy trình làm việc đề xuất

1. **Viết test cho hành vi mong muốn** (hoặc cho bug) → thấy nó lỗi.
2. **Viết/sửa mã** cho test qua.
3. **Chạy cục bộ:** `make test` (kèm sanitizer) — nhanh, trước khi commit.
4. **Đẩy lên** → CI chạy ma trận, sanitizer, phân tích tĩnh, coverage.
5. **Xem xét (review)** kết quả CI cùng mã trong pull request; chỉ merge khi xanh.
6. **Bổ sung test hồi quy** cho mọi lỗi tìm thấy sau đó.

Đây là **TDD (Test-Driven Development)** khi bạn viết test trước; không bắt buộc, nhưng thói quen "mỗi bug một test" thì nên có.

## 21.12. Lỗi thường gặp khi kiểm thử

| Sai lầm | Hậu quả | Cách tránh |
|---|---|---|
| Test phụ thuộc thứ tự chạy/trạng thái toàn cục | Lúc đạt lúc không | Mỗi test tự dựng và dọn dẹp; reset toàn cục |
| Test phụ thuộc giờ, mạng, số ngẫu nhiên không cố định | Mong manh (flaky) | Tiêm phụ thuộc; seed cố định |
| Không assert gì (chỉ gọi hàm) | Coverage cao nhưng vô dụng | Mỗi test phải kiểm tra kết quả |
| Test làm rò rỉ hoặc UB mà không ai thấy | Lỗi lọt | Luôn chạy với ASan/UBSan |
| Test quá lớn, kiểm tra nhiều thứ | Khó biết cái gì hỏng | Một ý một test, tên rõ ràng |
| Sửa test cho khớp mã sai | Che giấu lỗi | Xác nhận kỳ vọng từ yêu cầu, không từ kết quả hiện tại |
| So sánh `double` bằng `==` | Test sai | `ASSERT_NEAR` |
| Bỏ qua test lỗi ("để sau") | Nợ kỹ thuật | Sửa hoặc xóa; không để đỏ kéo dài |
| CI chỉ chạy một compiler/nền tảng | Lỗi di động lọt | Ma trận |
| CI quá chậm | Người ta bỏ qua | Cache, song song hóa, tách job nhanh/chậm |

## 21.13. Tóm tắt

- Kiểm thử tự động giúp phát hiện hồi quy, cho phép refactor an toàn; xây kim tự tháp: nhiều unit test nhanh, ít test tích hợp/đầu-cuối.
- Viết mã dễ kiểm thử: tách logic khỏi I/O, tránh toàn cục, tiêm phụ thuộc, tất định.
- Framework: tự làm/Unity (nhẹ), Check/Criterion (cô lập, đẹp), CMocka/CMock (mock); quan trọng là chạy tự động và trả mã thoát đúng.
- Mock bằng con trỏ hàm, link seam, `--wrap`; kiểm thử cả đường lỗi.
- Coverage (`gcov`/`lcov`) chỉ ra mã chưa được thử, không chứng minh test tốt.
- CI bằng GitHub Actions: build ma trận, ASan/UBSan, phân tích tĩnh, coverage; bảo vệ nhánh `main`.

## 21.14. Bài tập

1. Thêm bộ test cho module `Vec` (chương 9): kiểm tra `push` nhiều phần tử, tăng sức chứa, `pop` trên rỗng, và (dùng `--wrap=realloc`) hết bộ nhớ ở lần `push` thứ `k`. Chạy dưới ASan.
2. Viết bộ test cho `parse_int` (chương 13) gồm ít nhất 15 trường hợp biên; sau đó tạo test theo thuộc tính: với mọi `int` ngẫu nhiên, `parse_int(to_string(x)) == x`.
3. Cấu hình CTest cho một dự án nhiều module; thêm bài test đầu-cuối bằng script shell so đầu ra với "golden file".
4. Đo coverage của module bạn; tìm các nhánh chưa phủ; viết thêm test để phủ ít nhất một đường xử lý lỗi. Nhận xét: coverage tăng có nghĩa là gì và không có nghĩa là gì?
5. Viết `.github/workflows/ci.yml` với ma trận Ubuntu/macOS × gcc/clang, một job ASan+UBSan, một job `cppcheck`; cố ý commit một lỗi (use-after-free, cảnh báo) để thấy CI đỏ, rồi sửa.
6. Thêm fuzz target cho một parser của bạn vào CI như một job chạy 60 giây mỗi lần; lưu mọi đầu vào gây crash làm artifact.
7. Refactor một chương trình "khó test" (trộn I/O và logic) từ các chương trước thành lõi thuần + lớp I/O mỏng, rồi viết test cho lõi. Mô tả những gì bạn phải đổi.
8. Viết test dùng **đồng hồ giả** để kiểm tra một bộ đếm thời gian sống (TTL cache) mà không dùng `sleep`.
9. (Thử thách) Thiết lập CI có thêm job đo hiệu năng (chương 17) so sánh với lần chạy trước và **thất bại nếu chậm hơn quá 20%**; bàn về độ nhiễu của runner dùng chung và cách giảm nó (ví dụ dùng `callgrind` để đếm lệnh).

Mã nguồn mẫu: /code/chapter-21
