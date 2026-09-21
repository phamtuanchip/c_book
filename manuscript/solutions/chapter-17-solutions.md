# Chương 17 — Lời giải bài tập

> Kết quả thời gian phụ thuộc máy của bạn; hãy đo và so sánh **tỷ lệ**, không so số tuyệt đối. Luôn biên dịch với `-O2`.

## Bài 1: `count_duplicates` bằng ba cách

```c
// dup_count.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return (double)t.tv_sec + (double)t.tv_nsec * 1e-9; }

/* Gốc: O(n^2). Đếm số cặp (i<j) có a[i]==a[j]. */
static long dup_naive(const int *a, int n) {
    long c = 0;
    for (int i = 0; i < n; i++) for (int j = i + 1; j < n; j++) if (a[i] == a[j]) c++;
    return c;
}

static int cmp_int(const void *x, const void *y) { int a = *(const int *)x, b = *(const int *)y; return (a > b) - (a < b); }

/* Sắp xếp + quét: O(n log n). Nhóm k phần tử bằng nhau cho k(k-1)/2 cặp. */
static long dup_sort(const int *src, int n) {
    int *a = malloc((size_t)n * sizeof *a);
    if (!a) return -1;
    memcpy(a, src, (size_t)n * sizeof *a);
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

/* Mảng đếm: O(n + K) khi giá trị nằm trong [0, K). */
#define K 100000
static long dup_count(const int *a, int n) {
    int *cnt = calloc(K, sizeof *cnt);
    if (!cnt) return -1;
    long pairs = 0;
    for (int i = 0; i < n; i++) pairs += cnt[a[i]]++;       // cnt hiện có = số phần tử bằng a[i] đã gặp
    free(cnt);
    return pairs;
}

int main(void) {
    for (int n = 10000; n <= 1000000; n *= 10) {
        int *a = malloc((size_t)n * sizeof *a);
        srand(42);
        for (int i = 0; i < n; i++) a[i] = rand() % K;

        double t0 = now();
        long r1 = n <= 100000 ? dup_naive(a, n) : -1;        // O(n^2) quá chậm với 10^6
        double t1 = now();
        long r2 = dup_sort(a, n);
        double t2 = now();
        long r3 = dup_count(a, n);
        double t3 = now();
        printf("n=%7d naive %8.3fs sort %.4fs count %.4fs  %s\n", n, t1 - t0, t2 - t1, t3 - t2,
               (r1 < 0 || r1 == r2) && r2 == r3 ? "ket qua khop" : "KHAC NHAU!");
        free(a);
    }
    return 0;
}
```

Xu hướng: `naive` tăng ×100 mỗi khi `n` tăng ×10; `sort` tăng ~×12; `count` tăng ×10. Luôn **so kết quả** giữa các phiên bản trước khi so tốc độ.

## Bài 2: theo hàng và theo cột

Với `-O0`, `-O2`, `-O3` tỷ lệ cột/hàng thường ~5–15× khi `N = 4096` (ma trận 64 MB không vừa cache). Với `N = 256` (256 KB, vừa L2) chênh lệch gần như biến mất vì mọi dữ liệu nằm sẵn trong cache. Với `-O3` compiler đôi khi tự đổi thứ tự vòng lặp (*loop interchange*) — nếu thấy hai kết quả bằng nhau, hãy làm phép tính phụ thuộc giữa các vòng (ví dụ `sum = sum * 31 + m[...]`) để chặn tối ưu đó.

## Bài 3: AoS và SoA

Đo cập nhật một trường (`x += vx * dt`) trên 10 triệu hạt: SoA thường nhanh hơn 2–4× vì mỗi dòng cache 64 byte chứa 16 giá trị `float` hữu ích, so với AoS chỉ dùng 4/60 byte của mỗi struct. `perf stat -e cache-misses,cache-references ./prog` cho thấy số cache-miss của AoS lớn hơn nhiều. Nếu vòng lặp dùng **mọi** trường của mỗi hạt, chênh lệch nhỏ đi hoặc AoS thắng.

## Bài 4: dự đoán nhánh

Với mảng byte ngẫu nhiên 32 triệu phần tử: **ngẫu nhiên** chậm nhất (nhánh ~50% sai), **đã sắp xếp** nhanh 3–6×, **không nhánh** (`sum += ~t & data[i]` hoặc `sum += (data[i] >= 128) * data[i]`) có tốc độ ổn định, gần bằng bản đã sắp xếp. Xem assembly bằng `gcc -O2 -S`: với `-O2` mới, GCC thường tự chuyển bản có `if` thành lệnh `cmov` (không nhánh), làm chênh lệch biến mất — hãy so sánh cả `-O1`.

## Bài 5: mức tối ưu và cờ

Thường thấy: `-O0` chậm nhất (gấp 3–5 lần `-O2`); `-O2` và `-O3` gần bằng nhau; `-O2 -march=native` nhanh hơn với vòng lặp vector hóa được; `-flto` đáng kể khi nhiều file `.c` gọi nhau. `-O3` có thể **chậm hơn** `-O2` do phình mã (nhiều inlining/unrolling làm hỏng cache lệnh). Ghi lại kết quả thành bảng.

## Bài 6: danh sách liên kết và mảng

Duyệt tổng 10 triệu số: mảng nhanh hơn 3–10× (truy cập tuần tự, prefetch tốt; mỗi nút danh sách thêm 8 byte con trỏ và nằm rải rác trong heap nên mỗi bước có thể là một cache-miss). Chèn 1000 phần tử ở giữa: mảng phải `memmove` phần đuôi (O(n) mỗi lần) nhưng `memmove` cực nhanh trên bộ nhớ liền kề; danh sách O(1) khi **đã có con trỏ** tới vị trí nhưng để **tìm** vị trí đó phải duyệt O(n) qua các cache-miss. Kết quả nhiều khi bất ngờ: mảng vẫn thắng ở kích thước vài chục nghìn phần tử.

## Bài 7: `benchmark` tổng quát

Dùng `bench()` ở mục 17.3 với các hàm `memcpy`, vòng lặp `for`, `memmove` trên bộ đệm 64 MB. `memcpy` và `memmove` thường ngang nhau (thư viện dùng lệnh vector/`rep movsb`); vòng lặp tay với `-O2` được compiler nhận diện và đổi thành `memcpy` (kiểm tra bằng `objdump -d`), còn với `-O0` chậm hơn nhiều.

## Bài 8: `restrict` và vector hóa

```c
// restrict_demo.c
#include <stddef.h>

void add_norestrict(float *out, const float *a, const float *b, size_t n) {
    for (size_t i = 0; i < n; i++) out[i] = a[i] + b[i];
}

void add_restrict(float *restrict out, const float *restrict a, const float *restrict b, size_t n) {
    for (size_t i = 0; i < n; i++) out[i] = a[i] + b[i];
}
```

`gcc -O3 -fopt-info-vec restrict_demo.c -c`: bản `restrict` được báo `loop vectorized`; bản không có `restrict` compiler thường thêm **kiểm tra chồng lấn lúc chạy** (versioning) hoặc từ chối vector hóa vì `out` có thể trùng `a`/`b`. Lưu ý: nếu bạn gọi hàm `restrict` với vùng nhớ **chồng lấn** thì đó là UB.

## Bài 9: tối ưu bộ đếm từ 1 GB

Quy trình gợi ý (đo sau **mỗi** bước): (1) `perf record` để xác định hotspot — thường là `fgets`/`strtok` và băm; (2) đọc theo khối 1 MB bằng `fread` và tự tách từ; (3) bảng băm với hàm băm nhanh (FNV-1a) và `realloc` gộp; (4) tránh cấp phát từng từ (dùng arena); (5) song song: chia file theo khoảng byte (cắt ở ranh giới từ), mỗi luồng một bảng riêng rồi gộp. Kỳ vọng: từ vài chục giây xuống vài giây; trình bày bảng thời gian + `perf stat` từng bước.
