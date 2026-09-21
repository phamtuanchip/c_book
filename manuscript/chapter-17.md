# Chương 17 — Tối ưu hóa & profiling

## Mục tiêu chương

- Áp dụng quy trình đúng: **đo → tìm điểm nghẽn → tối ưu → đo lại**, thay vì đoán.
- Đo thời gian chính xác trong C (`clock_gettime`, `clock`) và tránh các sai lầm khi benchmark.
- Dùng **profiler**: `gprof`, `perf`, `valgrind --tool=callgrind`, cùng flame graph.
- Phân biệt **tối ưu thuật toán** (lợi lớn nhất) với **tối ưu vi mô** (cache, nhánh, inlining).
- Hiểu **bộ nhớ đệm (cache)**, **tính cục bộ dữ liệu**, và bố cục AoS/SoA; chứng minh bằng thí nghiệm.
- Biết các cờ tối ưu của compiler (`-O2`, `-O3`, `-march=native`, LTO, PGO) và cách kiểm tra kết quả.
- Nhận ra khi nào **không** nên tối ưu.

## 17.1. Nguyên tắc: đo trước khi tối ưu

Donald Knuth: *"Tối ưu hóa sớm là gốc rễ của mọi điều xấu"* (premature optimization is the root of all evil). Đầy đủ hơn, ông nói nên bỏ qua các tối ưu nhỏ trong khoảng 97% thời gian, nhưng **đừng bỏ lỡ 3% còn lại** — nơi mà chương trình thật sự tốn thời gian.

Bài học thực hành:

1. **Viết đúng và rõ ràng trước.** Chương trình đúng nhưng chậm có thể sửa; chương trình nhanh nhưng sai thì vô dụng.
2. **Xác định mục tiêu hiệu năng** cụ thể ("xử lý 1 triệu bản ghi dưới 2 giây"). Nếu đã đạt, dừng lại.
3. **Đo** để tìm **điểm nghẽn (hotspot)** thật sự. Trực giác của lập trình viên về chỗ chậm **thường sai**.
4. **Sửa một thứ mỗi lần** và **đo lại** để xác nhận cải thiện.
5. **Giữ kiểm thử** để chắc rằng tối ưu không làm sai kết quả.

Định luật Amdahl nhắc vì sao phải nhắm đúng chỗ: nếu một phần chiếm `p` tổng thời gian và bạn làm nó nhanh gấp `s` lần, tổng tốc độ chỉ tăng `1 / ((1-p) + p/s)`. Tăng tốc phần chiếm 5% thời gian lên vô hạn cũng chỉ giúp chương trình nhanh hơn ~5%.

### Hệ thống tầng thứ tự ưu tiên

Lợi ích lớn nhất đến từ trên xuống:

1. **Thuật toán và cấu trúc dữ liệu** (O(n²) → O(n log n)): hàng trăm–nghìn lần.
2. **Giảm công việc thừa** (tránh I/O, cấp phát, sao chép, tính lại): nhiều lần.
3. **Tính cục bộ bộ nhớ/cache**: vài lần.
4. **Cờ compiler** (`-O2`): thường 2–5 lần so với `-O0`.
5. **Vi tối ưu / SIMD / song song**: tùy chỗ.

## 17.2. Đo thời gian trong chương trình

### Các đồng hồ

| Hàm | Đo gì | Ghi chú |
|---|---|---|
| `clock()` (`<time.h>`) | Thời gian **CPU** của tiến trình | Chuẩn C; độ chính xác thấp; cộng dồn mọi luồng |
| `time()` | Giây thực (wall-clock) | Quá thô để benchmark |
| `clock_gettime(CLOCK_MONOTONIC, ...)` | Thời gian thực đơn điệu, ns | POSIX; **nên dùng** để đo khoảng thời gian |
| `timespec_get(&ts, TIME_UTC)` | Thời gian thực | C11, di động nhưng không đơn điệu |
| `QueryPerformanceCounter` | Đồng hồ độ phân giải cao | Windows |

**Thời gian thực (wall-clock)** là thời gian người dùng cảm nhận; **thời gian CPU** là thời gian CPU thực sự chạy mã của bạn (không tính lúc chờ I/O hoặc bị lập lịch ra). Với mã tính toán một luồng chúng gần bằng nhau; với I/O hoặc đa luồng thì khác nhau.

### Bộ đo tiện dụng

```c
// timer.h
#ifndef TIMER_H
#define TIMER_H

#define _POSIX_C_SOURCE 200809L
#include <time.h>

static inline double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

#endif
```

```c
double t0 = now_seconds();
do_work();
double t1 = now_seconds();
printf("mat %.3f ms\n", (t1 - t0) * 1000.0);
```

Nếu không có `clock_gettime` (Windows/MSVC), `clock()` là phương án di động (`(double)(clock() - start) / CLOCKS_PER_SEC`).

### Công cụ dòng lệnh: `time`

```bash
time ./prog
# real  0m1.234s    thời gian thực
# user  0m1.200s    thời gian CPU chạy mã của bạn
# sys   0m0.030s    thời gian CPU trong nhân hệ điều hành (I/O, cấp phát...)
```

`real` ≫ `user + sys` nghĩa là chương trình **chờ** (I/O, khóa, mạng); `sys` lớn nghĩa là nhiều lời gọi hệ thống.

## 17.3. Phương pháp benchmark đúng

Benchmark sai còn tệ hơn không có benchmark vì cho kết luận sai. Các nguyên tắc:

1. **Chạy nhiều lần** và lấy **trung vị/min**, kèm độ lệch, không lấy một lần duy nhất. Máy có nhiễu (tiến trình khác, tần số CPU thay đổi).
2. **Khởi động (warm-up):** lần chạy đầu chậm hơn do cache lạnh, bộ nhớ chưa được cấp, trang chưa nạp. Bỏ vài lần đầu.
3. **Dữ liệu thực tế và ổn định:** cùng một bộ dữ liệu (dùng seed cố định) cho mọi phương án; kích thước đủ lớn để không vừa trong cache nếu ứng dụng thật lớn.
4. **Chống compiler loại bỏ mã "vô dụng".** Nếu bạn tính kết quả mà không dùng, `-O2` có thể xóa hết vòng lặp và bạn đo... 0 giây. Luôn **dùng kết quả** (in ra, cộng vào biến `volatile`, hoặc trả về từ hàm được đánh dấu noinline).

```c
volatile long sink;          // ghi vào biến volatile buộc compiler giữ phép tính
long r = compute(data, n);
sink = r;
```

5. **Đo phần cần đo:** không tính thời gian tạo dữ liệu hay in ra màn hình.
6. **Biên dịch với cấu hình giống thật (`-O2`), không dùng `-O0`** để so sánh hiệu năng. Ngược lại, dùng `-O0 -g` để debug.
7. **Giảm nhiễu:** đóng ứng dụng khác, cắm điện laptop, cố định CPU (`taskset -c 2 ./prog` trên Linux), tránh chạy trong máy ảo khi cần độ chính xác cao.
8. **Đo cả bộ nhớ:** `/usr/bin/time -v ./prog` cho `Maximum resident set size`.
9. **So sánh công bằng:** cùng đầu vào, cùng cờ biên dịch, cùng độ chính xác kết quả.

Khung benchmark nhỏ:

```c
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

typedef void (*BenchFn)(void *ctx);

static int cmp_double(const void *a, const void *b) {
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

// Chạy fn `reps` lần, bỏ `warmup` lần đầu, in min/median
void bench(const char *name, BenchFn fn, void *ctx, int reps, int warmup) {
    double *t = malloc((size_t)reps * sizeof *t);
    if (!t) return;
    for (int i = 0; i < warmup; i++) fn(ctx);
    for (int i = 0; i < reps; i++) {
        double t0 = now_seconds();
        fn(ctx);
        t[i] = now_seconds() - t0;
    }
    qsort(t, (size_t)reps, sizeof *t, cmp_double);
    printf("%-24s min %8.3f ms   median %8.3f ms\n", name, t[0] * 1e3, t[reps / 2] * 1e3);
    free(t);
}
```

## 17.4. Profiling: tìm điểm nghẽn

**Profiler** cho biết chương trình dành thời gian ở **hàm/dòng** nào. Hai kiểu:

- **Lấy mẫu (sampling):** dừng chương trình định kỳ (hàng trăm–nghìn lần/giây) và ghi lại đang chạy hàm nào. Chi phí thấp, kết quả thống kê tốt (perf, gprof phần nào).
- **Đo vết (instrumentation):** chèn mã đếm/ghi khi mỗi hàm được gọi; chính xác về số lần gọi nhưng chậm và làm sai lệch (callgrind).

### Chương trình mẫu để profile

```c
// profile_demo.c — cố ý có điểm nghẽn
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Điểm nghẽn: đếm phần tử trùng bằng vòng lặp lồng nhau O(n^2)
static int count_duplicates(const int *a, int n) {
    int dups = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (a[i] == a[j]) dups++;
    return dups;
}

static long checksum(const int *a, int n) {
    long s = 0;
    for (int i = 0; i < n; i++) s += a[i];
    return s;
}

int main(void) {
    int n = 30000;
    int *a = malloc((size_t)n * sizeof *a);
    srand(42);
    for (int i = 0; i < n; i++) a[i] = rand() % 100000;

    printf("checksum = %ld\n", checksum(a, n));
    printf("duplicates = %d\n", count_duplicates(a, n));
    free(a);
    return 0;
}
```

### `gprof`

```bash
gcc -std=c11 -O2 -g -pg profile_demo.c -o profile_demo      # -pg: chèn mã đo
./profile_demo                                              # sinh gmon.out
gprof ./profile_demo gmon.out | head -20
```

Kết quả mẫu:

```text
Flat profile:
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 98.5      0.85     0.85        1   850.00   850.00  count_duplicates
  1.2      0.86     0.01        1    10.00    10.00  checksum
```

Cách đọc: `% time` phần trăm thời gian; `self` thời gian trong chính hàm đó (không tính hàm nó gọi); `calls` số lần gọi. Ở đây `count_duplicates` chiếm gần như toàn bộ thời gian — đó là điểm cần sửa. Hạn chế của `gprof`: bị lệch khi hàm được inline, độ phân giải thấp (0,01 s), tốt cho chương trình chạy đủ lâu.

### `perf` (Linux) — công cụ mạnh nhất

```bash
gcc -std=c11 -O2 -g -fno-omit-frame-pointer profile_demo.c -o profile_demo
perf stat ./profile_demo              # thống kê tổng: chu kỳ, lệnh, cache-miss, branch-miss
perf record -g ./profile_demo         # lấy mẫu kèm ngăn xếp gọi
perf report                           # giao diện duyệt kết quả
perf annotate count_duplicates        # xem từng dòng/lệnh assembly tốn bao nhiêu
```

`perf stat` xuất các chỉ số như:

```text
   1,234,567,890  cycles
   2,345,678,901  instructions      # 1.90 insn per cycle
       5,432,109  cache-misses
       1,234,567  branch-misses
```

**Số lệnh trên mỗi chu kỳ (IPC)** thấp (< 1) gợi ý chương trình **chờ bộ nhớ** (cache-miss); IPC cao (2–4) gợi ý CPU-bound. Đây là gợi ý để chọn hướng tối ưu.

### Callgrind + KCachegrind (Valgrind)

```bash
valgrind --tool=callgrind ./profile_demo
callgrind_annotate callgrind.out.<pid> | head -30
kcachegrind callgrind.out.<pid>       # giao diện đồ họa (Linux)
```

Cho **số lệnh chính xác** và **đồ thị gọi hàm**; chậm 20–50 lần nhưng **kết quả ổn định, lặp lại được** (không phụ thuộc nhiễu hệ thống) — rất tốt để so sánh trước/sau.

### Flame graph

Biểu diễn trực quan kết quả `perf record -g`: mỗi ô là một hàm, **độ rộng = tỷ lệ thời gian**, xếp chồng theo ngăn xếp gọi. Các "tảng" rộng là điểm nghẽn. (Công cụ của Brendan Gregg: `stackcollapse-perf.pl`, `flamegraph.pl`.)

### Trên Windows/macOS

- macOS: **Instruments** (Time Profiler), hoặc `sample <pid>`.
- Windows: **Visual Studio Performance Profiler**, **Windows Performance Analyzer**, hoặc chạy trong **WSL2** để dùng `perf`.

### Tìm điểm nghẽn bộ nhớ

Ngoài thời gian, hãy đo bộ nhớ: `valgrind --tool=massif ./prog` (theo dõi heap theo thời gian, `ms_print massif.out.<pid>`); `heaptrack` cho công cụ trực quan; AddressSanitizer cũng cho thống kê.

### Thí nghiệm: sửa điểm nghẽn bằng thuật toán tốt hơn

Với `count_duplicates` ở trên (O(n²)), n = 30.000 mất ~0,9 giây. Cách thay đổi thuật toán: **sắp xếp** rồi đếm các phần tử liền kề bằng nhau — O(n log n):

```c
static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

// Đếm số CẶP (i<j) có a[i]==a[j]: nhóm k phần tử bằng nhau đóng góp k*(k-1)/2 cặp
static long count_duplicates_fast(const int *src, int n) {
    int *a = malloc((size_t)n * sizeof *a);
    if (!a) return -1;
    memcpy(a, src, (size_t)n * sizeof *a);        // giữ nguyên mảng gốc
    qsort(a, (size_t)n, sizeof *a, cmp_int);

    long pairs = 0;
    for (int i = 0; i < n; ) {
        int j = i;
        while (j < n && a[j] == a[i]) j++;
        long k = j - i;
        pairs += k * (k - 1) / 2;
        i = j;
    }
    free(a);
    return pairs;
}
```

Với n = 30.000: từ ~0,9 s xuống vài mili giây (nhanh hơn ~200 lần). **Đây là điều mà cờ compiler hay vi tối ưu không thể làm được.** Kiểm chứng rằng hai hàm cho **cùng kết quả** trên nhiều đầu vào.

Một cách khác, O(n) trung bình: dùng **bảng băm** đếm tần suất (chương 10). Bảng so sánh:

| Cách | Thời gian | Bộ nhớ thêm |
|---|---|---|
| Vòng lặp lồng nhau | O(n²) | O(1) |
| Sắp xếp + quét | O(n log n) | O(n) (bản sao) |
| Bảng băm | O(n) trung bình | O(n) |
| Nếu giá trị nằm trong khoảng nhỏ `[0, K)`: mảng đếm | O(n + K) | O(K) |

Lựa chọn tùy ràng buộc, ví dụ khi giá trị chỉ trong `0..99999` thì mảng đếm `int count[100000]` là nhanh và đơn giản nhất.

## 17.5. Bộ nhớ đệm (cache) và tính cục bộ

### Vấn đề: CPU nhanh hơn RAM rất nhiều

Truy cập RAM mất ~100 ns (hàng trăm chu kỳ); truy cập thanh ghi/cache L1 chỉ ~1 ns. CPU dùng các **tầng cache** (L1 ~32 KB, L2 ~256 KB–1 MB, L3 nhiều MB) chứa dữ liệu gần đây.

```text
Thanh ghi  → L1 cache → L2 → L3 → RAM → Đĩa
  nhanh nhất ─────────────────────────► chậm nhất, lớn nhất
```

Dữ liệu được chuyển giữa RAM và cache theo **dòng cache (cache line)**, thường **64 byte**. Khi bạn đọc một `int`, cả 64 byte xung quanh cũng được nạp. Vì vậy:

- **Cục bộ không gian (spatial locality):** truy cập các địa chỉ **liền kề** rất rẻ — dữ liệu đã nằm sẵn trong dòng cache.
- **Cục bộ thời gian (temporal locality):** dùng lại dữ liệu vừa dùng — nó còn nằm trong cache.

**Cache-miss** (dữ liệu không có trong cache) là một trong những nguyên nhân hàng đầu khiến chương trình chậm hơn kỳ vọng nhiều lần dù số phép tính như nhau.

### Thí nghiệm: duyệt ma trận theo hàng và theo cột

C lưu ma trận **theo hàng** (row-major). Duyệt theo hàng đi qua bộ nhớ **liên tiếp**; duyệt theo cột **nhảy cóc** mỗi bước cách nhau `cols * sizeof(int)` byte.

```c
// cache_demo.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

#define N 4096

int main(void) {
    int *m = malloc((size_t)N * N * sizeof *m);        // 64 MB (4096*4096*4)
    if (!m) return 1;
    for (size_t i = 0; i < (size_t)N * N; i++) m[i] = 1;

    volatile long sink;
    double t0 = now_seconds();
    long sum = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            sum += m[(size_t)i * N + j];               // theo hàng: liên tiếp
    sink = sum;
    double t_row = now_seconds() - t0;

    t0 = now_seconds();
    sum = 0;
    for (int j = 0; j < N; j++)
        for (int i = 0; i < N; i++)
            sum += m[(size_t)i * N + j];               // theo cột: nhảy cóc N*4 byte
    sink = sum;
    double t_col = now_seconds() - t0;

    printf("theo hang: %.1f ms\ntheo cot : %.1f ms (cham hon %.1fx)\n",
           t_row * 1e3, t_col * 1e3, t_col / t_row);
    free(m);
    return 0;
}
```

Kết quả điển hình với `-O2`: **theo cột chậm hơn 5–20 lần** dù thực hiện đúng cùng số phép cộng. Chỉ đổi thứ tự hai vòng lặp là một tối ưu miễn phí. (Compiler đôi khi tự đổi thứ tự — *loop interchange* — nên nếu bạn thấy hai kết quả bằng nhau, thử `-O0` hoặc thêm phép phụ thuộc.)

### Bố cục dữ liệu: AoS và SoA

**AoS (Array of Structures)** — mảng các struct — cách viết tự nhiên:

```c
struct ParticleAoS { float x, y, z; float vx, vy, vz; int id; char name[32]; };
struct ParticleAoS ps[N];
// muốn cập nhật x của mọi hạt: mỗi lần đọc kéo cả 60+ byte của struct vào cache, chỉ dùng 4 byte
```

**SoA (Structure of Arrays)** — mỗi trường một mảng:

```c
struct ParticlesSoA {
    float *x, *y, *z;
    float *vx, *vy, *vz;
};
// cập nhật x: đọc x[i] liên tiếp, mỗi dòng cache chứa 16 giá trị x hữu ích
for (int i = 0; i < n; i++) p.x[i] += p.vx[i] * dt;
```

Khi vòng lặp nóng chỉ dùng **một vài trường**, SoA dùng cache hiệu quả hơn nhiều và cho phép compiler dùng **SIMD** (xử lý nhiều số một lệnh). Khi bạn thường dùng **mọi trường của một phần tử cùng lúc**, AoS lại tốt hơn. Đây là quyết định phụ thuộc mẫu truy cập — hãy đo.

### Các kỹ thuật thân thiện cache khác

- **Thu nhỏ dữ liệu:** dùng kiểu nhỏ nhất đủ dùng (`uint8_t`, `int32_t`), sắp xếp trường struct để giảm padding (chương 10). Nhiều phần tử/dòng cache hơn.
- **Cấp phát liên tục:** mảng thay vì danh sách liên kết. Duyệt danh sách liên kết nhảy giữa các địa chỉ ngẫu nhiên gây cache-miss ở mỗi nút; `Vec` (mảng động) thường thắng danh sách liên kết cả khi chèn/xóa giữa.
- **Chia khối (blocking/tiling):** với ma trận lớn, xử lý theo khối nhỏ vừa cache thay vì chạy hết hàng/cột.
- **Tránh false sharing** (đa luồng): hai luồng ghi vào hai biến **khác nhau nhưng cùng dòng cache** khiến dòng bị chuyển qua lại giữa các nhân. Đệm mỗi biến ra 64 byte (`_Alignas(64)`).
- **Prefetch:** `__builtin_prefetch(&a[i + 16])` gợi ý CPU nạp trước; hiếm khi cần vì phần cứng đã tự prefetch cho mẫu truy cập tuần tự.

## 17.6. Dự đoán nhánh và mã không nhánh

CPU thực hiện lệnh theo **đường ống (pipeline)** và **đoán trước** kết quả của mỗi `if`. Đoán sai (branch misprediction) tốn ~15–20 chu kỳ. Với điều kiện **ngẫu nhiên**, dự đoán rất khó.

Thí nghiệm kinh điển: tổng các phần tử ≥ 128 trong mảng byte **ngẫu nhiên** so với mảng **đã sắp xếp**:

```c
for (int i = 0; i < n; i++)
    if (data[i] >= 128) sum += data[i];
```

Sau khi **sắp xếp** mảng, vòng lặp có thể nhanh hơn 3–6 lần vì điều kiện trở nên dự đoán được (một đoạn đầu toàn sai, đoạn sau toàn đúng) — mặc dù số phép tính như nhau. Với dữ liệu ngẫu nhiên, có thể viết **không nhánh**:

```c
int t = (data[i] - 128) >> 31;      // -1 nếu data[i] < 128, ngược lại 0 (dịch số có dấu)
sum += ~t & data[i];
```

Compiler thường tự chuyển sang lệnh `cmov` (di chuyển có điều kiện) khi có thể. Đây là **vi tối ưu** — đo trước khi làm.

## 17.7. Cờ tối ưu của compiler

### Các mức

| Cờ | Ý nghĩa |
|---|---|
| `-O0` | Không tối ưu (mặc định). Nhanh biên dịch, dễ debug. |
| `-O1` | Tối ưu cơ bản. |
| `-O2` | **Mức khuyến nghị cho bản phát hành.** Nhiều tối ưu an toàn. |
| `-O3` | Thêm inline mạnh, vector hóa vòng lặp, unroll; có thể **phình mã**; không luôn nhanh hơn `-O2`. |
| `-Os` | Tối ưu kích thước mã. |
| `-Og` | Tối ưu vừa đủ mà vẫn debug được. |
| `-Ofast` | `-O3` + bỏ qua các quy tắc số thực chặt chẽ (`-ffast-math`): **có thể đổi kết quả tính toán** (NaN, thứ tự cộng); chỉ dùng khi hiểu rõ hậu quả. |

### Các cờ hữu ích khác

```bash
-march=native         # dùng mọi lệnh CPU hiện tại (AVX2...). Bản nhị phân KHÔNG chạy trên CPU cũ hơn!
-flto                 # tối ưu lúc liên kết (Link-Time Optimization): inline xuyên file .c
-funroll-loops        # trải vòng lặp
-fno-omit-frame-pointer   # giữ frame pointer để profiler đọc được ngăn xếp gọi
-fprofile-generate / -fprofile-use   # PGO: tối ưu theo dữ liệu chạy thực
-DNDEBUG              # tắt assert
```

### Tối ưu theo hồ sơ chạy (PGO)

```bash
gcc -O2 -fprofile-generate prog.c -o prog          # 1) biên dịch bản đo
./prog < workload.txt                              # 2) chạy với dữ liệu điển hình -> sinh .gcda
gcc -O2 -fprofile-use prog.c -o prog               # 3) biên dịch lại dùng số liệu thực
```

Compiler biết nhánh nào hay chạy, hàm nào nóng, nên bố trí mã và inline tốt hơn: thường cải thiện 5–30%.

### Kiểm chứng compiler đã làm gì

```bash
gcc -O2 -S -fverbose-asm prog.c -o prog.s     # xem assembly
gcc -O2 -fopt-info-vec-optimized prog.c       # in các vòng lặp đã được vector hóa
gcc -O2 -fopt-info-inline-missed prog.c       # các chỗ inline thất bại
```

Trang **godbolt.org** (Compiler Explorer) cho phép xem assembly của mã C ngay trên web với nhiều compiler/cờ — tuyệt vời để học.

### Giúp compiler tối ưu

- **`const`** và **`restrict`** cho con trỏ không chồng lấn: `void add(float *restrict out, const float *restrict a, const float *restrict b, size_t n)` cho phép vector hóa vì compiler biết `out` không trùng `a`, `b`.
- **`static`** cho hàm nội bộ để compiler inline dễ hơn.
- **`static inline`** cho hàm nhỏ trong header.
- **Giữ vòng lặp nóng đơn giản:** không gọi hàm chưa biết, không có tác dụng phụ phức tạp, số lần lặp và bước nhảy rõ ràng.
- Dùng **`__builtin_expect`** (`likely`/`unlikely`) chỉ khi profile chứng minh.
- **Tránh UB:** tràn số có dấu khiến compiler suy diễn sai; dùng biến chỉ số kiểu `size_t`/`int` nhất quán.

## 17.8. Các tối ưu phổ biến ở mức mã

| Vấn đề | Tối ưu |
|---|---|
| `strlen(s)` gọi trong điều kiện vòng lặp: `for (i = 0; i < strlen(s); i++)` — O(n²) | Tính một lần: `size_t n = strlen(s);` |
| Tính lại biểu thức bất biến trong vòng lặp | Đưa ra ngoài vòng lặp (compiler đôi khi tự làm, nhưng không luôn) |
| Chia/lấy dư cho số không đổi 2ⁿ | `x >> n`, `x & (2^n - 1)` cho số **không dấu** (compiler đã tự làm với hằng số) |
| Cấp phát nhỏ lặp lại trong vòng nóng | Cấp phát một lần, tái sử dụng; arena/pool |
| Nối chuỗi lặp lại bằng `strcat` (mỗi lần quét từ đầu) | Giữ con trỏ cuối chuỗi hoặc dùng bộ đệm động |
| I/O từng byte/dòng nhỏ | Dùng bộ đệm lớn (`setvbuf`), đọc theo khối, `fread` cả file |
| `printf` trong vòng lặp nóng | Gom vào bộ đệm, in một lần |
| Truyền struct lớn theo giá trị | Truyền con trỏ `const` |
| Tìm tuyến tính trong tập lớn | Bảng băm / cây / sắp xếp + tìm nhị phân |
| `qsort` với hàm so sánh qua con trỏ hàm (không inline) | Tự viết sắp xếp cho kiểu cụ thể (đo trước) |

Ví dụ lỗi O(n²) ẩn:

```c
for (size_t i = 0; i < strlen(s); i++) {   // strlen chạy lại mỗi vòng: tổng O(n²)
    if (s[i] == ' ') count++;
}
```

Compiler đôi khi giấu bớt nếu chứng minh `s` không đổi, nhưng đừng dựa vào đó.

## 17.9. So sánh thuật toán sắp xếp — thí nghiệm

```c
// sort_bench.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "timer.h"

static int cmp_int(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

// Sắp xếp chèn: O(n^2), tốt cho mảng nhỏ / gần sắp xếp
static void insertion_sort(int *a, size_t n) {
    for (size_t i = 1; i < n; i++) {
        int k = a[i]; size_t j = i;
        while (j > 0 && a[j - 1] > k) { a[j] = a[j - 1]; j--; }
        a[j] = k;
    }
}

// Sắp xếp nhanh (quicksort), chọn pivot giữa, đệ quy trên phần nhỏ hơn để giới hạn độ sâu
static void quicksort(int *a, long lo, long hi) {
    while (lo < hi) {
        int pivot = a[lo + (hi - lo) / 2];
        long i = lo, j = hi;
        while (i <= j) {
            while (a[i] < pivot) i++;
            while (a[j] > pivot) j--;
            if (i <= j) { int t = a[i]; a[i] = a[j]; a[j] = t; i++; j--; }
        }
        if (j - lo < hi - i) { quicksort(a, lo, j); lo = i; }     // đệ quy phần nhỏ, lặp phần lớn
        else                 { quicksort(a, i, hi); hi = j; }
    }
}

// Sắp xếp trộn (mergesort): O(n log n) ổn định, cần bộ nhớ phụ
static void merge_rec(int *a, int *tmp, size_t n) {
    if (n < 2) return;
    size_t mid = n / 2;
    merge_rec(a, tmp, mid);
    merge_rec(a + mid, tmp, n - mid);
    size_t i = 0, j = mid, k = 0;
    while (i < mid && j < n) tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    while (i < mid) tmp[k++] = a[i++];
    while (j < n)   tmp[k++] = a[j++];
    memcpy(a, tmp, n * sizeof *a);
}
static void mergesort_ints(int *a, size_t n) {
    int *tmp = malloc(n * sizeof *tmp);
    if (!tmp) return;
    merge_rec(a, tmp, n);
    free(tmp);
}

static int is_sorted(const int *a, size_t n) {
    for (size_t i = 1; i < n; i++) if (a[i - 1] > a[i]) return 0;
    return 1;
}

int main(void) {
    size_t n = 2000000;
    int *orig = malloc(n * sizeof *orig), *work = malloc(n * sizeof *work);
    if (!orig || !work) return 1;
    srand(12345);                                        // seed cố định -> dữ liệu lặp lại được
    for (size_t i = 0; i < n; i++) orig[i] = rand();

    struct { const char *name; } algos[] = { {"qsort (libc)"}, {"quicksort"}, {"mergesort"} };
    for (int k = 0; k < 3; k++) {
        memcpy(work, orig, n * sizeof *work);            // dữ liệu giống hệt nhau cho mọi thuật toán
        double t0 = now_seconds();
        if (k == 0) qsort(work, n, sizeof *work, cmp_int);
        else if (k == 1) quicksort(work, 0, (long)n - 1);
        else mergesort_ints(work, n);
        double dt = now_seconds() - t0;
        printf("%-14s %8.1f ms  %s\n", algos[k].name, dt * 1e3, is_sorted(work, n) ? "OK" : "SAI!");
    }
    (void)insertion_sort;                                // dùng cho thử nghiệm mảng nhỏ ở bài tập
    free(orig); free(work);
    return 0;
}
```

Điều thường thấy: `quicksort` chuyên biệt cho `int` nhanh hơn `qsort` của libc (vì hàm so sánh được inline, không gọi qua con trỏ hàm); mergesort ổn định nhưng dùng thêm bộ nhớ. Nhưng **hãy tự đo trên máy bạn**, đừng tin số liệu trong sách.

## 17.10. Khi nào KHÔNG nên tối ưu?

- **Khi chưa đo.** Đừng tối ưu chỗ bạn "cảm thấy" chậm.
- **Khi đã đạt mục tiêu hiệu năng.**
- **Khi làm giảm đáng kể độ dễ đọc** để đổi lấy vài phần trăm ở nơi không nóng.
- **Khi tối ưu bằng cách bật `-Ofast`/`-ffast-math`** mà không hiểu hậu quả số thực.
- **Khi chưa có kiểm thử**: tối ưu thường sinh bug tinh vi.
- **Khi bottleneck nằm ngoài CPU** (mạng, đĩa, cơ sở dữ liệu): tối ưu vòng lặp C vô nghĩa.

Nếu buộc phải làm mã khó đọc hơn để nhanh hơn, hãy **giữ phiên bản đơn giản** làm chuẩn kiểm tra và **ghi chú** vì sao.

## 17.11. Lỗi thường gặp khi tối ưu

| Sai lầm | Hậu quả | Cách tránh |
|---|---|---|
| Đo với `-O0` rồi kết luận | Kết luận vô nghĩa | Đo với `-O2` |
| Kết quả không được dùng | Compiler xóa mã, đo ra ~0 | Dùng kết quả (`volatile sink`) |
| Đo một lần duy nhất | Nhiễu | Nhiều lần, lấy min/median |
| Bỏ qua khởi động | Lần đầu chậm | Warm-up |
| Tối ưu chỗ không nóng | Mất công vô ích | Profile trước |
| Đổi thuật toán mà không so kết quả | Sai kết quả | Kiểm thử tương đương |
| `-march=native` cho bản phát hành | Crash trên máy khác | Chỉ dùng cho build cục bộ |
| So sánh dữ liệu khác nhau giữa các phương án | Kết luận sai | Cùng dữ liệu, cùng seed |
| Tin lý thuyết mà không đo (ví dụ "danh sách liên kết nhanh hơn mảng khi chèn") | Chậm hơn thực tế do cache | Đo |

## 17.12. Tóm tắt

- Quy trình: **đo → tìm hotspot → sửa → đo lại**; đừng tối ưu trước khi có số liệu.
- Đo bằng `clock_gettime(CLOCK_MONOTONIC)`, benchmark nhiều lần với dữ liệu cố định, dùng kết quả để không bị compiler xóa.
- Profiler: `gprof` (đơn giản), `perf` (mạnh, Linux), `callgrind` (chính xác, chậm), flame graph (trực quan).
- **Thuật toán/cấu trúc dữ liệu** mang lại lợi ích lớn nhất; sau đó là **giảm công việc thừa**, **cache/tính cục bộ**, rồi mới đến cờ compiler và vi tối ưu.
- Duyệt bộ nhớ liên tiếp nhanh hơn nhiều; SoA thân thiện cache khi chỉ dùng vài trường; tránh false sharing.
- `-O2` cho bản phát hành; `-O3`, `-march=native`, LTO, PGO có thể giúp nhưng phải đo; `-Ofast` thay đổi ngữ nghĩa số thực.

## 17.13. Bài tập

1. Chạy `profile_demo.c` với `gprof` và `perf`; xác định hàm nóng. Viết `count_duplicates_fast` bằng ba cách (sắp xếp, mảng đếm, bảng băm), kiểm chứng kết quả giống nhau và lập bảng thời gian với n = 10⁴, 10⁵, 10⁶.
2. Chạy `cache_demo.c` với `-O0`, `-O2`, `-O3`; ghi lại tỷ lệ theo cột / theo hàng. Thử kích thước `N` nhỏ (256) và lớn (8192) và giải thích khác biệt theo kích thước cache.
3. Viết thí nghiệm AoS và SoA: cập nhật một trường của 10 triệu hạt; đo thời gian và (trên Linux) `perf stat -e cache-misses`.
4. Viết thí nghiệm dự đoán nhánh: tổng phần tử ≥ 128 với mảng ngẫu nhiên, mảng đã sắp xếp và phiên bản không nhánh. Kiểm tra mã assembly bằng `gcc -S` hoặc godbolt.
5. Đo chênh lệch `-O0`, `-O1`, `-O2`, `-O3`, `-O2 -march=native`, `-O2 -flto` cho `sort_bench.c`. Bản nào nhanh nhất? Có nơi nào `-O3` chậm hơn `-O2` không?
6. So sánh duyệt **danh sách liên kết** và **mảng** cho 10 triệu số nguyên (tổng các phần tử), cùng đo chèn 1000 phần tử ở giữa. Giải thích kết quả.
7. Viết `benchmark` tổng quát (như 17.3) và dùng nó so sánh `memcpy` với vòng lặp sao chép tay và với `memmove`.
8. Dùng `restrict` và `-O3 -fopt-info-vec` để làm compiler vector hóa `add(out, a, b, n)`; so sánh thời gian có và không có `restrict`.
9. (Thử thách) Tối ưu chương trình đếm tần suất từ (chương 12) cho file 1 GB: đo, tìm hotspot bằng `perf`, cải thiện I/O (đọc khối lớn), bảng băm, và (tùy chọn) song song hóa bằng luồng (chương 15). Trình bày số liệu trước/sau từng bước.

Mã nguồn mẫu: /code/chapter-17
