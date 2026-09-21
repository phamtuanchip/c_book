# Chương 15 — Lời giải bài tập

> Biên dịch: `gcc -std=c11 -Wall -Wextra -g -pthread prog.c -o prog`. Chạy trên Linux, macOS hoặc WSL.

## Bài 1: sửa race bằng mutex và bằng nguyên tử

```c
// counter_variants.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <time.h>

#define N 1000000
#define THREADS 4

static long g_plain;                                       // KHÔNG an toàn: kết quả sai
static long g_locked;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static atomic_long g_atomic;

static void *work_plain(void *a)  { (void)a; for (int i = 0; i < N; i++) g_plain++; return NULL; }
static void *work_mutex(void *a)  { (void)a; for (int i = 0; i < N; i++) { pthread_mutex_lock(&g_lock); g_locked++; pthread_mutex_unlock(&g_lock); } return NULL; }
static void *work_atomic(void *a) { (void)a; for (int i = 0; i < N; i++) atomic_fetch_add(&g_atomic, 1); return NULL; }

static double run(void *(*fn)(void *)) {
    pthread_t t[THREADS];
    struct timespec a, b;
    clock_gettime(CLOCK_MONOTONIC, &a);
    for (int i = 0; i < THREADS; i++) pthread_create(&t[i], NULL, fn, NULL);
    for (int i = 0; i < THREADS; i++) pthread_join(t[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &b);
    return (double)(b.tv_sec - a.tv_sec) + (double)(b.tv_nsec - a.tv_nsec) * 1e-9;
}

int main(void) {
    double t1 = run(work_plain), t2 = run(work_mutex), t3 = run(work_atomic);
    printf("khong khoa : %ld (mong doi %ld) %.3fs\n", g_plain, (long)N * THREADS, t1);
    printf("mutex      : %ld %.3fs\n", g_locked, t2);
    printf("atomic     : %ld %.3fs\n", (long)g_atomic, t3);
    return 0;
}
```

Kết quả điển hình: bản không khóa sai (nhỏ hơn 4.000.000, thay đổi mỗi lần); mutex đúng nhưng chậm nhất; atomic đúng và nhanh hơn mutex vài lần. Chạy dưới `-fsanitize=thread` để thấy TSan báo `data race` ở `g_plain`.

## Bài 2: giảm số lần khóa

```c
// counter_local.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>

#define N 10000000
static long g_total;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static void *work(void *arg) {
    (void)arg;
    long local = 0;
    for (int i = 0; i < N; i++) local++;             // cộng vào biến CỤC BỘ: không tranh chấp
    pthread_mutex_lock(&g_lock);                     // chỉ khóa MỘT lần
    g_total += local;
    pthread_mutex_unlock(&g_lock);
    return NULL;
}

int main(void) {
    pthread_t t[4];
    for (int i = 0; i < 4; i++) pthread_create(&t[i], NULL, work, NULL);
    for (int i = 0; i < 4; i++) pthread_join(t[i], NULL);
    printf("%ld\n", g_total);                        // 40000000
    return 0;
}
```

Nhanh hơn hẳn bản khóa mỗi lần tăng, gần như tuyến tính theo số nhân — bài học: **giảm chia sẻ** tốt hơn tối ưu khóa.

## Bài 3: tổng mảng lớn bằng `N` luồng

Dùng `parallel_sum.c` (mục 15.2), đọc `N` từ `argv[1]`, cấp phát `pthread_t` và `SumTask` động (`calloc`), chia đoạn `[i*chunk, (i+1)*chunk)` và luồng cuối nhận phần dư. Đo speed-up = thời gian 1 luồng / thời gian `N` luồng; sẽ **bão hòa** khi `N` vượt số nhân vật lý, và với phép cộng đơn giản thường bị giới hạn bởi **băng thông bộ nhớ** hơn là CPU.

## Bài 4: thử phá `producer_consumer.c`

- Đổi `while` thành `if` quanh `pthread_cond_wait`: với nhiều consumer, một consumer bị đánh thức có thể thấy hàng đợi đã rỗng (bị luồng khác lấy mất) hoặc bị đánh thức giả rồi đọc phần tử không hợp lệ → **kết quả sai** hoặc `count` âm (trên `size_t` thành số rất lớn).
- Bỏ `pthread_cond_broadcast` trong `bq_close`: các consumer đang chờ ở `not_empty` **ngủ mãi**, `pthread_join` treo — chương trình không kết thúc.
- TSan/Helgrind: phiên bản đúng không báo gì; các phiên bản đã bị phá thường bị báo race hoặc (Helgrind) cảnh báo dùng cond var không đúng khóa.

## Bài 5: producer–consumer bằng semaphore

```c
// pc_semaphore.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#define CAP 4
#define ITEMS 20

static int buf[CAP];
static int head, tail;
static sem_t empty_slots, full_slots;                       // đếm chỗ trống / phần tử có sẵn
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;    // bảo vệ head/tail

static void *producer(void *a) {
    (void)a;
    for (int i = 0; i < ITEMS; i++) {
        sem_wait(&empty_slots);                             // chờ có chỗ trống
        pthread_mutex_lock(&lock);
        buf[tail] = i; tail = (tail + 1) % CAP;
        pthread_mutex_unlock(&lock);
        sem_post(&full_slots);                              // báo có phần tử
    }
    return NULL;
}

static void *consumer(void *a) {
    long *sum = a;
    for (int i = 0; i < ITEMS; i++) {
        sem_wait(&full_slots);
        pthread_mutex_lock(&lock);
        int v = buf[head]; head = (head + 1) % CAP;
        pthread_mutex_unlock(&lock);
        sem_post(&empty_slots);
        *sum += v;
    }
    return NULL;
}

int main(void) {
    sem_init(&empty_slots, 0, CAP);
    sem_init(&full_slots, 0, 0);
    long sum = 0;
    pthread_t p, c;
    pthread_create(&p, NULL, producer, NULL);
    pthread_create(&c, NULL, consumer, &sum);
    pthread_join(p, NULL);
    pthread_join(c, NULL);
    printf("tong = %ld (mong doi %d)\n", sum, ITEMS * (ITEMS - 1) / 2);   // 190
    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    return 0;
}
```

(Trên macOS, semaphore không tên `sem_init` không được hỗ trợ; dùng `sem_open` hoặc `dispatch_semaphore`.)

## Bài 6: tái hiện và chẩn đoán deadlock

```c
// deadlock.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <unistd.h>

static pthread_mutex_t A = PTHREAD_MUTEX_INITIALIZER, B = PTHREAD_MUTEX_INITIALIZER;

static void *t1(void *x) { (void)x; pthread_mutex_lock(&A); sleep(1); pthread_mutex_lock(&B); pthread_mutex_unlock(&B); pthread_mutex_unlock(&A); return NULL; }
static void *t2(void *x) { (void)x; pthread_mutex_lock(&B); sleep(1); pthread_mutex_lock(&A); pthread_mutex_unlock(&A); pthread_mutex_unlock(&B); return NULL; }

int main(void) {
    pthread_t a, b;
    pthread_create(&a, NULL, t1, NULL);
    pthread_create(&b, NULL, t2, NULL);
    pthread_join(a, NULL);        // treo mãi
    pthread_join(b, NULL);
    return 0;
}
```

Chẩn đoán: `gdb -p $(pidof deadlock)` rồi `thread apply all bt` — hai luồng đứng ở `pthread_mutex_lock`, mỗi luồng giữ khóa kia cần. Sửa: cả hai luồng khóa theo cùng thứ tự (A rồi B).

## Bài 7: thread pool

Khung: một `Queue` các công việc `{ void (*fn)(void *); void *arg; }` bảo vệ bởi mutex + cond `not_empty`; `N` worker lặp `lấy → chạy`. `pool_submit` bỏ công việc vào queue (và `signal`); `pool_shutdown` đặt cờ `closed`, `broadcast`, rồi `join` mọi worker sau khi queue rỗng — cùng mẫu `bq_get/bq_close` ở mục 15.6 và `FdQueue` ở chương 20. Điểm cần kiểm thử: nộp 1000 việc rồi `shutdown` → cả 1000 việc đều chạy; gọi `submit` sau `shutdown` phải trả lỗi; ASan/TSan sạch.

## Bài 8: bữa tối của triết gia (không deadlock)

Đánh số 5 cây đũa 0..4; triết gia `i` cần đũa `i` và `(i+1)%5`. **Luôn lấy đũa có số nhỏ hơn trước**:

```c
// philosophers.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>

#define N 5
static pthread_mutex_t fork_lock[N];

static void *philosopher(void *arg) {
    int id = (int)(long)arg;
    int a = id, b = (id + 1) % N;
    if (a > b) { int t = a; a = b; b = t; }        // thứ tự khóa nhất quán => không có chu trình chờ
    for (int meal = 0; meal < 3; meal++) {
        pthread_mutex_lock(&fork_lock[a]);
        pthread_mutex_lock(&fork_lock[b]);
        printf("triet gia %d an (bua %d)\n", id, meal + 1);
        pthread_mutex_unlock(&fork_lock[b]);
        pthread_mutex_unlock(&fork_lock[a]);
    }
    return NULL;
}

int main(void) {
    pthread_t t[N];
    for (int i = 0; i < N; i++) pthread_mutex_init(&fork_lock[i], NULL);
    for (long i = 0; i < N; i++) pthread_create(&t[i], NULL, philosopher, (void *)i);
    for (int i = 0; i < N; i++) pthread_join(t[i], NULL);
    for (int i = 0; i < N; i++) pthread_mutex_destroy(&fork_lock[i]);
    return 0;
}
```

Nếu mỗi triết gia lấy đũa trái rồi phải, cả 5 cùng lấy đũa trái sẽ deadlock; đánh số thứ tự phá "chờ vòng tròn".

## Bài 9: đếm từ song song

Ba phương án (đo trên file ≥ 100 MB): (1) **một khóa toàn cục** cho bảng băm — đúng nhưng các luồng chờ nhau, gần như không tăng tốc; (2) **khóa theo bucket** — tranh chấp giảm mạnh, tăng tốc tốt khi từ phân tán đều; (3) **bảng riêng mỗi luồng** rồi **gộp** ở cuối — không khóa trong lúc đếm, thường nhanh nhất, đổi lại tốn bộ nhớ và bước gộp. Đây là ví dụ của nguyên tắc: *tránh chia sẻ* thắng *chia sẻ có đồng bộ*.
