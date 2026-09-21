# Chương 15 — Đa luồng & đồng bộ (POSIX threads)

## Mục tiêu chương

- Hiểu **tiến trình** và **luồng (thread)** khác nhau ra sao, và khi nào nên dùng đa luồng.
- Tạo và chờ luồng bằng `pthread_create`/`pthread_join`; truyền tham số và nhận kết quả an toàn.
- Hiểu **race condition** bằng ví dụ có thể tái hiện, và sửa bằng **mutex**.
- Dùng **biến điều kiện (condition variable)** để cài đặt mẫu producer–consumer.
- Nhận diện và tránh **deadlock**; biết về khóa đọc–ghi, thao tác nguyên tử (`stdatomic.h`), thread-local.
- Dùng **ThreadSanitizer** và **Helgrind** để bắt lỗi đồng bộ.
- Biết tương đương trên Windows và C11 `<threads.h>`.

> **Môi trường:** mã trong chương dùng POSIX threads (Linux, macOS, WSL, hoặc MSYS2/MinGW có `winpthreads`). Biên dịch với `-pthread`: `gcc -std=c11 -Wall -Wextra -g -pthread prog.c -o prog`. Trong `-std=c11` nghiêm ngặt có thể cần `#define _POSIX_C_SOURCE 200809L` ở đầu file.

## 15.1. Tiến trình và luồng

- **Tiến trình (process):** một chương trình đang chạy, có **không gian địa chỉ riêng** (bộ nhớ, file mở...). Các tiến trình cách ly nhau; giao tiếp qua IPC (pipe, socket, shared memory).
- **Luồng (thread):** một dòng thực thi **bên trong tiến trình**. Các luồng của cùng tiến trình **dùng chung** bộ nhớ heap, biến toàn cục, file mở; mỗi luồng có **stack và thanh ghi riêng**.

```text
Tiến trình
┌───────────────────────────────────────────────┐
│ Mã, biến toàn cục, heap, file mở  (CHIA SẺ)    │
│                                               │
│  Luồng 1        Luồng 2        Luồng 3        │
│  ┌──────┐       ┌──────┐       ┌──────┐       │
│  │stack │       │stack │       │stack │       │  (RIÊNG)
│  │thanh │       │thanh │       │thanh │       │
│  │ghi   │       │ghi   │       │ghi   │       │
│  └──────┘       └──────┘       └──────┘       │
└───────────────────────────────────────────────┘
```

Lợi ích của đa luồng:

1. **Tận dụng nhiều nhân CPU** (song song hóa tính toán).
2. **Không bị chặn:** một luồng chờ I/O trong khi luồng khác tiếp tục (giao diện phản hồi, server xử lý nhiều client).
3. **Chia sẻ dữ liệu dễ** (nhanh hơn IPC giữa các tiến trình).

Cái giá: **bộ nhớ chung** nghĩa là **có thể xảy ra lỗi đồng bộ** — nhóm lỗi khó tái hiện và khó gỡ nhất. Quy tắc: *chỉ dùng đa luồng khi thật sự cần, và giữ dữ liệu chia sẻ ở mức tối thiểu*.

### Đồng thời (concurrency) và song song (parallelism)

- **Đồng thời:** nhiều việc được **tiến triển xen kẽ** (có thể trên một nhân).
- **Song song:** nhiều việc **chạy cùng lúc** trên nhiều nhân.

Thứ tự luồng chạy do **bộ lập lịch của hệ điều hành** quyết định, bạn **không kiểm soát** và nó **thay đổi giữa các lần chạy**. Đó là gốc rễ của mọi vấn đề bên dưới.

## 15.2. Tạo và chờ luồng

```c
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg);
int pthread_join(pthread_t thread, void **retval);
```

- `pthread_create`: tạo luồng mới chạy hàm `start_routine(arg)`. Trả `0` nếu thành công, **mã lỗi** (không phải `-1`/`errno`) nếu thất bại.
- Hàm luồng có chữ ký cố định: `void *fn(void *arg)`.
- `pthread_join`: chờ luồng kết thúc và lấy giá trị trả về.

```c
// hello_threads.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>

static void *worker(void *arg) {
    int id = *(int *)arg;
    printf("Xin chao tu luong %d\n", id);
    return NULL;
}

int main(void) {
    pthread_t threads[4];
    int ids[4];

    for (int i = 0; i < 4; i++) {
        ids[i] = i;                                   // mỗi luồng một biến riêng (xem cảnh báo bên dưới)
        int rc = pthread_create(&threads[i], NULL, worker, &ids[i]);
        if (rc != 0) { fprintf(stderr, "pthread_create: %d\n", rc); return 1; }
    }
    for (int i = 0; i < 4; i++) pthread_join(threads[i], NULL);

    printf("xong\n");
    return 0;
}
```

Chạy nhiều lần bạn sẽ thấy **thứ tự dòng in ra khác nhau** — đúng như đã nói.

### Truyền tham số: cẩn thận về vòng đời

Lỗi rất phổ biến: truyền địa chỉ của biến **có thể hết hiệu lực** trước khi luồng đọc nó.

```c
for (int i = 0; i < 4; i++) {
    pthread_create(&threads[i], NULL, worker, &i);    // SAI: mọi luồng đọc cùng biến i, mà i đang thay đổi
}
```

Các luồng có thể thấy `i = 4` hoặc giá trị bất kỳ. Sửa bằng cách:

1. Mỗi luồng một **biến riêng còn sống** khi luồng chạy (mảng `ids[i]` như trên, phải tồn tại đến khi `join`).
2. Hoặc **cấp phát struct trên heap** và để luồng `free` (chuyển quyền sở hữu).
3. Tuyệt đối **không truyền địa chỉ biến cục bộ của hàm đã return**.

### Truyền nhiều tham số và nhận kết quả bằng struct

```c
typedef struct {
    int    id;
    const int *data;
    size_t start, end;      // đoạn [start, end) mà luồng xử lý
    long   partial_sum;     // kết quả
} SumTask;

static void *sum_worker(void *arg) {
    SumTask *t = arg;
    long s = 0;
    for (size_t i = t->start; i < t->end; i++) s += t->data[i];
    t->partial_sum = s;         // ghi vào struct của CHÍNH luồng này: không tranh chấp
    return NULL;
}
```

Ví dụ hoàn chỉnh: cộng một mảng lớn bằng nhiều luồng, **mỗi luồng ghi vào ô riêng** — không cần khóa:

```c
// parallel_sum.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NTHREADS 4

typedef struct {
    const int *data;
    size_t start, end;
    long partial_sum;
} SumTask;

static void *sum_worker(void *arg) {
    SumTask *t = arg;
    long s = 0;
    for (size_t i = t->start; i < t->end; i++) s += t->data[i];
    t->partial_sum = s;
    return NULL;
}

int main(void) {
    const size_t N = 10000000;
    int *data = malloc(N * sizeof *data);
    if (!data) return 1;
    for (size_t i = 0; i < N; i++) data[i] = 1;

    pthread_t th[NTHREADS];
    SumTask   tasks[NTHREADS];
    size_t chunk = N / NTHREADS;

    for (int i = 0; i < NTHREADS; i++) {
        tasks[i].data  = data;
        tasks[i].start = i * chunk;
        tasks[i].end   = (i == NTHREADS - 1) ? N : (i + 1) * chunk;   // luồng cuối nhận phần dư
        tasks[i].partial_sum = 0;
        if (pthread_create(&th[i], NULL, sum_worker, &tasks[i]) != 0) { perror("create"); return 1; }
    }

    long total = 0;
    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(th[i], NULL);              // join tạo "hàng rào bộ nhớ": kết quả luồng đã ghi là nhìn thấy được
        total += tasks[i].partial_sum;
    }
    printf("tong = %ld (mong doi %zu)\n", total, N);
    free(data);
    return 0;
}
```

Đây là mẫu **tốt nhất** để dùng đa luồng: **chia dữ liệu thành phần không giao nhau**, mỗi luồng xử lý phần của mình, rồi **gộp kết quả** sau `join`. Khi không có dữ liệu chia sẻ có thể ghi, không cần khóa.

### Trả về giá trị từ luồng

```c
static void *compute(void *arg) {
    long *result = malloc(sizeof *result);
    if (!result) return NULL;
    *result = 42;
    return result;                  // trả con trỏ tới heap (KHÔNG trả con trỏ tới biến cục bộ)
}

void *ret;
pthread_join(tid, &ret);
if (ret) { printf("%ld\n", *(long *)ret); free(ret); }
```

### `detach` và vòng đời luồng

Mỗi luồng tạo ra **phải được `pthread_join` hoặc `pthread_detach`**, nếu không tài nguyên của nó không được giải phóng (rò rỉ). `pthread_detach(tid)` cho luồng "tự dọn dẹp khi kết thúc" và không thể join.

Luồng kết thúc khi: hàm luồng `return`, gọi `pthread_exit(retval)`, hoặc bị hủy. Nếu **luồng chính** thoát (`return` từ `main`/`exit`) thì **toàn bộ tiến trình kết thúc**, kể cả các luồng khác đang chạy — vì vậy hãy `join` trước khi thoát.

## 15.3. Race condition — điều gì xảy ra khi không đồng bộ

**Race condition (điều kiện chạy đua):** kết quả phụ thuộc vào **thứ tự thực hiện xen kẽ** của các luồng khi nhiều luồng truy cập cùng dữ liệu và ít nhất một luồng **ghi**.

### Ví dụ có thể tái hiện

```c
// race.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>

static long counter = 0;               // biến chia sẻ

static void *inc(void *arg) {
    (void)arg;
    for (int i = 0; i < 1000000; i++) {
        counter++;                     // KHÔNG nguyên tử!
    }
    return NULL;
}

int main(void) {
    pthread_t a, b;
    pthread_create(&a, NULL, inc, NULL);
    pthread_create(&b, NULL, inc, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    printf("counter = %ld (mong doi 2000000)\n", counter);
    return 0;
}
```

Chạy vài lần: bạn sẽ nhận các giá trị như `1187342`, `1354021`... **luôn nhỏ hơn 2.000.000 và khác nhau mỗi lần**.

### Vì sao?

`counter++` trông là một phép toán nhưng CPU thực hiện **ba bước**: (1) đọc `counter` vào thanh ghi, (2) cộng 1, (3) ghi lại. Hai luồng có thể xen kẽ:

```text
Thời gian │ Luồng A            │ Luồng B            │ counter
──────────┼────────────────────┼────────────────────┼────────
    1     │ đọc  (thấy 5)      │                    │   5
    2     │                    │ đọc  (thấy 5)      │   5
    3     │ cộng 1 → 6         │                    │   5
    4     │                    │ cộng 1 → 6         │   5
    5     │ ghi 6              │                    │   6
    6     │                    │ ghi 6              │   6   ← mất một lần tăng!
```

Hai lần tăng nhưng kết quả chỉ tăng 1: **cập nhật bị mất (lost update)**. Vùng mã mà nhiều luồng không được cùng lúc vào gọi là **vùng găng (critical section)**.

Race condition **nguy hiểm** vì: xảy ra ngẫu nhiên, có thể biến mất khi thêm `printf` để gỡ lỗi (Heisenbug), có thể chỉ xuất hiện trên máy khách hàng nhiều nhân, và luôn là hành vi không xác định theo chuẩn C11 (*data race*).

## 15.4. Mutex — khóa loại trừ tương hỗ

**Mutex (mutual exclusion)** là khóa: tại một thời điểm **chỉ một luồng** giữ nó. Luồng khác gọi `lock` sẽ **bị chặn** cho đến khi luồng giữ khóa gọi `unlock`.

```c
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;     // khởi tạo tĩnh
// hoặc: pthread_mutex_init(&m, NULL); ... pthread_mutex_destroy(&m);

pthread_mutex_lock(&m);
    /* vùng găng: chỉ một luồng vào cùng lúc */
pthread_mutex_unlock(&m);
```

Sửa ví dụ trên:

```c
static long counter = 0;
static pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

static void *inc(void *arg) {
    (void)arg;
    for (int i = 0; i < 1000000; i++) {
        pthread_mutex_lock(&counter_lock);
        counter++;
        pthread_mutex_unlock(&counter_lock);
    }
    return NULL;
}
```

Kết quả luôn là `2000000`.

### Quy tắc dùng mutex

1. **Mỗi dữ liệu chia sẻ được bảo vệ bởi một mutex xác định.** Ghi rõ trong mã (comment "counter được bảo vệ bởi counter_lock"). Mọi truy cập — **cả đọc lẫn ghi** — đều phải giữ khóa.
2. **Giữ khóa càng ngắn càng tốt.** Không làm I/O, không `sleep`, không gọi hàm lạ trong khi giữ khóa.
3. **Luôn `unlock` trên mọi đường thoát**, kể cả đường lỗi (dễ quên khi có `return`/`goto` giữa `lock` và `unlock`).
4. Không `lock` một mutex thường **hai lần** trong cùng luồng (deadlock với chính mình); nếu cần, dùng mutex đệ quy (`PTHREAD_MUTEX_RECURSIVE`) nhưng nên tránh bằng thiết kế tốt.
5. Không `unlock` mutex mà bạn không giữ.
6. Kiểm tra giá trị trả về khi cần độ tin cậy cao (`pthread_mutex_lock` có thể trả `EINVAL`, `EDEADLK`...).

### Đóng gói dữ liệu cùng mutex của nó

Cách tốt nhất để không quên khóa: đặt mutex **cùng struct** với dữ liệu và chỉ truy cập qua hàm:

```c
typedef struct {
    pthread_mutex_t lock;
    long            value;
} Counter;

void counter_init(Counter *c)          { pthread_mutex_init(&c->lock, NULL); c->value = 0; }
void counter_destroy(Counter *c)       { pthread_mutex_destroy(&c->lock); }

void counter_add(Counter *c, long n) {
    pthread_mutex_lock(&c->lock);
    c->value += n;
    pthread_mutex_unlock(&c->lock);
}

long counter_get(Counter *c) {
    pthread_mutex_lock(&c->lock);
    long v = c->value;
    pthread_mutex_unlock(&c->lock);
    return v;
}
```

### Chi phí của khóa và cách giảm

Khóa làm luồng **chờ nhau**, giảm song song. Nếu mọi luồng liên tục tranh cùng một khóa, chương trình đa luồng có thể chậm hơn đơn luồng. Cách giảm:

- **Gộp công việc cục bộ:** mỗi luồng cộng vào biến cục bộ, chỉ khóa **một lần** ở cuối để cộng vào tổng chung.
- Dùng cấu trúc **không chia sẻ**: mỗi luồng ghi phần riêng (như `parallel_sum`).
- Dùng thao tác **nguyên tử** (15.8) cho bộ đếm/cờ đơn giản.
- Chia nhỏ khóa (nhiều khóa cho nhiều phần dữ liệu) — cẩn thận deadlock.

## 15.5. Biến điều kiện (condition variable)

Mutex giải quyết "không vào cùng lúc". **Biến điều kiện** giải quyết "**chờ một sự kiện** xảy ra": luồng ngủ cho đến khi luồng khác báo hiệu điều kiện đã thay đổi — thay vì **chờ bận (busy-wait)** lãng phí CPU.

```c
pthread_cond_t c = PTHREAD_COND_INITIALIZER;

int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m);   // nhả m, ngủ, khi thức dậy khóa lại m
int pthread_cond_signal(pthread_cond_t *c);                     // đánh thức một luồng đang chờ
int pthread_cond_broadcast(pthread_cond_t *c);                  // đánh thức tất cả
```

### Mẫu chuẩn — luôn dùng **vòng lặp `while`** kiểm tra điều kiện

```c
pthread_mutex_lock(&m);
while (!condition) {                   // KHÔNG dùng if
    pthread_cond_wait(&c, &m);
}
/* condition đúng và ta đang giữ m */
pthread_mutex_unlock(&m);
```

Vì sao `while` mà không phải `if`?

1. **Đánh thức giả (spurious wakeup):** `pthread_cond_wait` có thể trả về mà không ai `signal`.
2. **Cạnh tranh:** giữa lúc bị đánh thức và lúc giành lại được khóa, luồng khác có thể đã lấy mất thứ bạn cần, nên phải kiểm tra lại.

Và bắt buộc: điều kiện được **bảo vệ bởi cùng mutex**; người báo hiệu phải **thay đổi điều kiện khi giữ mutex** rồi `signal`.

## 15.6. Producer–Consumer với bộ đệm giới hạn

Bài toán kinh điển: **producer** tạo dữ liệu bỏ vào hàng đợi; **consumer** lấy ra xử lý. Hàng đợi có sức chứa giới hạn: producer phải **chờ khi đầy**, consumer phải **chờ khi rỗng**.

Ta dùng queue vòng (chương 10), thêm một mutex và **hai** biến điều kiện: `not_full`, `not_empty`.

```c
// producer_consumer.c
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define CAP 8
#define NPRODUCERS 2
#define NCONSUMERS 3
#define ITEMS_PER_PRODUCER 10

typedef struct {
    int buf[CAP];
    size_t head, count;
    int closed;                       // producer báo "không còn dữ liệu nữa"
    pthread_mutex_t lock;
    pthread_cond_t  not_full, not_empty;
} BoundedQueue;

static void bq_init(BoundedQueue *q) {
    q->head = q->count = 0;
    q->closed = 0;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_full, NULL);
    pthread_cond_init(&q->not_empty, NULL);
}

static void bq_destroy(BoundedQueue *q) {
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->not_full);
    pthread_cond_destroy(&q->not_empty);
}

// Thêm phần tử; chờ nếu đầy
static void bq_put(BoundedQueue *q, int v) {
    pthread_mutex_lock(&q->lock);
    while (q->count == CAP)                          // đầy -> chờ
        pthread_cond_wait(&q->not_full, &q->lock);
    q->buf[(q->head + q->count) % CAP] = v;
    q->count++;
    pthread_cond_signal(&q->not_empty);              // báo: hết rỗng
    pthread_mutex_unlock(&q->lock);
}

// Lấy phần tử; chờ nếu rỗng. Trả 1 nếu lấy được, 0 nếu hàng đợi đã đóng và hết dữ liệu.
static int bq_get(BoundedQueue *q, int *out) {
    pthread_mutex_lock(&q->lock);
    while (q->count == 0 && !q->closed)              // rỗng và còn khả năng có thêm -> chờ
        pthread_cond_wait(&q->not_empty, &q->lock);
    if (q->count == 0) {                             // rỗng và đã đóng -> hết
        pthread_mutex_unlock(&q->lock);
        return 0;
    }
    *out = q->buf[q->head];
    q->head = (q->head + 1) % CAP;
    q->count--;
    pthread_cond_signal(&q->not_full);               // báo: hết đầy
    pthread_mutex_unlock(&q->lock);
    return 1;
}

// Báo đóng: đánh thức tất cả consumer đang chờ để chúng thoát
static void bq_close(BoundedQueue *q) {
    pthread_mutex_lock(&q->lock);
    q->closed = 1;
    pthread_cond_broadcast(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

typedef struct { BoundedQueue *q; int id; } ProducerArg;
typedef struct { BoundedQueue *q; int id; long sum; } ConsumerArg;

static void *producer(void *arg) {
    ProducerArg *a = arg;
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int v = a->id * 1000 + i;
        bq_put(a->q, v);
    }
    return NULL;
}

static void *consumer(void *arg) {
    ConsumerArg *a = arg;
    int v;
    while (bq_get(a->q, &v)) {
        a->sum += v;                               // sum là riêng của luồng này -> không cần khóa
    }
    return NULL;
}

int main(void) {
    BoundedQueue q;
    bq_init(&q);

    pthread_t prod[NPRODUCERS], cons[NCONSUMERS];
    ProducerArg pa[NPRODUCERS];
    ConsumerArg ca[NCONSUMERS];

    for (int i = 0; i < NCONSUMERS; i++) {
        ca[i] = (ConsumerArg){ &q, i, 0 };
        pthread_create(&cons[i], NULL, consumer, &ca[i]);
    }
    for (int i = 0; i < NPRODUCERS; i++) {
        pa[i] = (ProducerArg){ &q, i + 1 };
        pthread_create(&prod[i], NULL, producer, &pa[i]);
    }

    for (int i = 0; i < NPRODUCERS; i++) pthread_join(prod[i], NULL);   // chờ producer xong
    bq_close(&q);                                                       // rồi mới đóng queue
    for (int i = 0; i < NCONSUMERS; i++) pthread_join(cons[i], NULL);

    long total = 0;
    for (int i = 0; i < NCONSUMERS; i++) total += ca[i].sum;

    long expected = 0;
    for (int p = 1; p <= NPRODUCERS; p++)
        for (int i = 0; i < ITEMS_PER_PRODUCER; i++) expected += p * 1000 + i;

    printf("tong nhan duoc = %ld, mong doi = %ld -> %s\n", total, expected, total == expected ? "DUNG" : "SAI");
    bq_destroy(&q);
    return 0;
}
```

Các điểm mấu chốt:

- **Một mutex bảo vệ toàn bộ trạng thái** của queue (`buf`, `head`, `count`, `closed`).
- **Hai biến điều kiện** tách "không đầy" và "không rỗng" để đánh thức đúng loại luồng.
- Mọi `wait` nằm trong vòng `while`.
- **Cách kết thúc sạch sẽ:** đặt cờ `closed` và `broadcast` để consumer thoát khi hết dữ liệu. Thiếu bước này, consumer sẽ ngủ mãi (chương trình treo).
- Thứ tự kết thúc quan trọng: `join` producer, `close`, rồi `join` consumer.

## 15.7. Deadlock (khóa chết)

**Deadlock** xảy ra khi hai (hoặc nhiều) luồng **chờ nhau mãi mãi**, mỗi luồng giữ một khóa mà luồng kia cần.

```c
static pthread_mutex_t A = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t B = PTHREAD_MUTEX_INITIALIZER;

void *t1(void *x) {
    pthread_mutex_lock(&A);
    pthread_mutex_lock(&B);        // chờ B
    /* ... */
    pthread_mutex_unlock(&B); pthread_mutex_unlock(&A);
    return NULL;
}

void *t2(void *x) {
    pthread_mutex_lock(&B);
    pthread_mutex_lock(&A);        // chờ A
    /* ... */
    pthread_mutex_unlock(&A); pthread_mutex_unlock(&B);
    return NULL;
}
```

```text
Luồng 1: giữ A ──chờ──► B
                  ▲        │
                  │        ▼
Luồng 2: chờ ◄── A ◄─── giữ B       → vòng chờ, không ai tiến được
```

Deadlock cần đủ **bốn điều kiện Coffman**: loại trừ tương hỗ, giữ-và-chờ, không chiếm quyền, **chờ vòng tròn**. Phá bất kỳ điều kiện nào là ngăn được deadlock. Cách thực tế nhất:

1. **Thứ tự khóa nhất quán (lock ordering):** quy ước toàn dự án — luôn khóa A trước B. Có thể sắp theo địa chỉ mutex khi cần khóa hai đối tượng cùng loại:

```c
void lock_pair(pthread_mutex_t *x, pthread_mutex_t *y) {
    if (x == y) { pthread_mutex_lock(x); return; }
    if (x < y) { pthread_mutex_lock(x); pthread_mutex_lock(y); }
    else       { pthread_mutex_lock(y); pthread_mutex_lock(x); }
}
```

2. **Giữ ít khóa nhất có thể** và **không gọi hàm chưa biết** (có thể lấy khóa khác) khi đang giữ khóa.
3. **`pthread_mutex_trylock`:** thử khóa không chặn; nếu không được thì nhả khóa đang giữ và thử lại (tránh giữ-và-chờ).
4. **Khóa có thời hạn:** `pthread_mutex_timedlock`.
5. Thiết kế **giảm nhu cầu chia sẻ** (truyền thông điệp qua queue thay vì nhiều khóa).

Các lỗi đồng bộ khác:

- **Livelock:** luồng liên tục thay đổi trạng thái để tránh nhau nhưng không tiến triển.
- **Starvation (đói):** một luồng không bao giờ được lấy khóa.
- **Priority inversion:** luồng ưu tiên thấp giữ khóa khiến luồng ưu tiên cao bị chặn.

## 15.8. Thao tác nguyên tử (`<stdatomic.h>`, C11)

Với dữ liệu đơn giản (bộ đếm, cờ), có thể dùng **kiểu nguyên tử** — CPU bảo đảm thao tác không bị xen ngang — không cần mutex.

```c
#include <stdatomic.h>

static atomic_long counter = 0;
static atomic_bool stop = false;

// trong luồng:
atomic_fetch_add(&counter, 1);            // tăng nguyên tử, trả về giá trị cũ
long v = atomic_load(&counter);
atomic_store(&stop, true);
```

`atomic_long counter; counter++;` cũng nguyên tử. Bộ đếm race ở trên sửa bằng một dòng đổi kiểu và không cần khóa. Nhanh hơn mutex nhiều cho thao tác đơn lẻ.

Hạn chế: chỉ bảo vệ **một biến**; nếu bất biến liên quan **nhiều biến**, vẫn cần mutex. Thứ tự bộ nhớ (`memory_order_*`) là chủ đề nâng cao; mặc định `memory_order_seq_cst` an toàn nhất.

`volatile` **không** thay thế nguyên tử/khóa cho đồng bộ luồng.

## 15.9. Các công cụ đồng bộ khác

### Khóa đọc–ghi (`pthread_rwlock_t`)

Nhiều luồng đọc đồng thời, nhưng ghi phải độc quyền:

```c
pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_rdlock(&rw);  /* đọc */  pthread_rwlock_unlock(&rw);
pthread_rwlock_wrlock(&rw);  /* ghi  */  pthread_rwlock_unlock(&rw);
```

Hữu ích khi đọc nhiều hơn ghi rất nhiều; nhưng có chi phí riêng nên đo trước khi dùng.

### Semaphore (`<semaphore.h>`, POSIX)

Bộ đếm đồng bộ: `sem_wait` giảm (chờ nếu 0), `sem_post` tăng. Dùng để giới hạn số luồng dùng một tài nguyên hoặc báo hiệu sự kiện.

### `pthread_once` và dữ liệu cục bộ luồng

```c
static pthread_once_t once = PTHREAD_ONCE_INIT;
static void init_once(void) { /* khởi tạo đúng một lần dù nhiều luồng gọi */ }
pthread_once(&once, init_once);
```

**Thread-local storage:** mỗi luồng có bản sao riêng của biến — hết tranh chấp:

```c
static _Thread_local int tls_counter = 0;      // C11 (hoặc __thread trên gcc)
```

Ví dụ điển hình: `errno` là thread-local.

### Barrier (`pthread_barrier_t`)

Cho mọi luồng chờ nhau tại một điểm rồi cùng tiếp tục — hữu ích cho tính toán song song theo giai đoạn.

## 15.10. Hàm an toàn với luồng (thread-safe)

Một hàm **thread-safe** cho kết quả đúng khi nhiều luồng gọi đồng thời. Những thứ **không** thread-safe:

- Hàm dùng **biến tĩnh/toàn cục nội bộ**: `strtok`, `localtime`, `asctime`, `rand`, `strerror`.
- Hàm trả con trỏ tới bộ đệm tĩnh.

Dùng bản **reentrant**: `strtok_r`, `localtime_r`, `rand_r`, `strerror_r`. Và nhớ: các hàm thư viện chuẩn như `printf`, `malloc` là thread-safe (nội bộ có khóa), nhưng **chuỗi các lời gọi** không nguyên tử: hai luồng in cùng lúc có thể trộn lẫn *giữa các lời gọi* (mỗi lời gọi `printf` riêng lẻ thì không bị xé nhỏ).

## 15.11. Công cụ gỡ lỗi đa luồng

### ThreadSanitizer (TSan)

```bash
gcc -std=c11 -g -O1 -fsanitize=thread -pthread race.c -o race
./race
```

Báo cáo mẫu:

```text
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 8 at 0x000000601040 by thread T2:
    #0 inc race.c:9
  Previous write of size 8 at 0x000000601040 by thread T1:
    #0 inc race.c:9
  Location is global 'counter' of size 8 at 0x000000601040 (race+0x...)
```

TSan cho biết **hai truy cập xung đột, biến nào, luồng nào, dòng nào**. Nó rất hiệu quả; nên chạy toàn bộ kiểm thử đa luồng dưới TSan. (Không dùng chung với `-fsanitize=address` trong cùng bản build.)

### Helgrind (Valgrind)

```bash
valgrind --tool=helgrind ./prog
```

Phát hiện race và cả **lỗi thứ tự khóa** (tiềm ẩn deadlock). Chậm hơn TSan nhưng không cần biên dịch lại.

### Gỡ lỗi deadlock

Khi chương trình **treo**, dùng `gdb -p <pid>` rồi `thread apply all bt` để xem mọi luồng đang chờ ở đâu; hai luồng cùng ở `pthread_mutex_lock` giữ khóa của nhau là dấu hiệu.

### Mẹo thiết kế

- **Hạn chế dữ liệu chia sẻ có thể ghi**; ưu tiên truyền thông điệp (queue) và dữ liệu bất biến.
- **Tài liệu hóa** khóa nào bảo vệ dữ liệu nào và thứ tự khóa.
- Kiểm thử với **số luồng lớn hơn số nhân** và bằng vòng lặp dài để tăng xác suất lộ lỗi; chạy nhiều lần, dưới TSan.
- Đừng "sửa" race bằng `sleep`.

## 15.12. Trên Windows và C11 `<threads.h>`

| Khái niệm | POSIX | Windows | C11 `<threads.h>` |
|---|---|---|---|
| Tạo luồng | `pthread_create` | `CreateThread`, `_beginthreadex` | `thrd_create` |
| Chờ luồng | `pthread_join` | `WaitForSingleObject` | `thrd_join` |
| Mutex | `pthread_mutex_t` | `CRITICAL_SECTION`, `HANDLE` mutex | `mtx_t` |
| Biến điều kiện | `pthread_cond_t` | `CONDITION_VARIABLE` | `cnd_t` |

Ví dụ C11 (đa nền tảng khi thư viện hỗ trợ; MSVC và glibc mới có, một số nơi như macOS thì chưa):

```c
#include <threads.h>
#include <stdio.h>

static int worker(void *arg) {
    printf("luong: %d\n", *(int *)arg);
    return 0;                             // kiểu trả về là int, khác pthread
}

int main(void) {
    thrd_t t;
    int v = 7;
    thrd_create(&t, worker, &v);
    thrd_join(t, NULL);
    return 0;
}
```

Nếu cần chạy trên Windows với MinGW-w64, `winpthreads` cho phép dùng chính mã pthread; hoặc dùng WSL. Với mã thật sự đa nền tảng, xem xét thư viện bọc như *tinycthread* hoặc CMake `find_package(Threads)`.

## 15.13. Lỗi thường gặp

| Lỗi | Hậu quả | Cách tránh |
|---|---|---|
| Truy cập dữ liệu chung không có khóa | Race, kết quả sai/ngẫu nhiên | Mutex hoặc nguyên tử; TSan |
| Truyền địa chỉ biến cục bộ/biến vòng lặp cho luồng | Luồng đọc giá trị sai | Biến riêng còn sống, hoặc struct trên heap |
| Quên `join`/`detach` | Rò rỉ tài nguyên luồng | Luôn join hoặc detach |
| `main` thoát khi luồng còn chạy | Luồng bị giết giữa chừng | Join trước khi thoát |
| `if` thay `while` với `cond_wait` | Bỏ lỡ điều kiện, đánh thức giả | Luôn `while` |
| Gọi `cond_wait` không giữ mutex | Hành vi không xác định | Khóa mutex trước |
| Quên `unlock` trên đường lỗi | Treo toàn hệ thống | Mẫu cleanup, kiểm tra kỹ |
| Khóa theo thứ tự khác nhau | Deadlock | Thứ tự khóa nhất quán |
| Dùng hàm không thread-safe (`strtok`) | Dữ liệu lẫn lộn | Bản `_r` |
| Giữ khóa khi làm I/O/`sleep` | Hiệu năng kém, deadlock | Giữ khóa ngắn |
| Tin `volatile` là đủ | Vẫn race | Nguyên tử/mutex |

## 15.14. Tóm tắt

- Luồng cùng tiến trình chia sẻ bộ nhớ, có stack riêng; thứ tự chạy không xác định.
- Tạo bằng `pthread_create`, chờ bằng `pthread_join`; truyền tham số sao cho còn hiệu lực khi luồng chạy.
- **Race condition** xảy ra khi có truy cập ghi chung không đồng bộ; sửa bằng **mutex** (giữ ngắn, mọi đường thoát đều unlock) hoặc **nguyên tử**.
- **Biến điều kiện** để chờ sự kiện, luôn trong vòng `while` và gắn với mutex; mẫu producer–consumer dùng hai biến điều kiện và cờ đóng.
- **Deadlock** tránh bằng thứ tự khóa nhất quán, giữ ít khóa, `trylock`.
- Kiểm tra bằng **TSan/Helgrind**; ưu tiên thiết kế ít chia sẻ.

## 15.15. Bài tập

1. Chạy `race.c` nhiều lần, ghi lại kết quả; sửa bằng mutex, sau đó bằng `atomic_long`; đo và so sánh thời gian (dùng `time`) cho 4 luồng × 10 triệu lần tăng.
2. Sửa `race.c` để mỗi luồng cộng vào biến cục bộ và chỉ khóa **một lần** ở cuối. So sánh thời gian với các phiên bản trước.
3. Viết chương trình tính tổng một mảng lớn bằng `N` luồng (`N` từ dòng lệnh) theo `parallel_sum`, và đo speed-up so với 1 luồng.
4. Chạy `producer_consumer.c` dưới ThreadSanitizer và Helgrind. Sau đó thử **cố ý** bỏ `while` thành `if` hoặc bỏ `broadcast` để xem chương trình treo/sai thế nào.
5. Viết thêm phiên bản producer–consumer với **`sem_t`** thay cho biến điều kiện.
6. Viết chương trình tái hiện **deadlock** bằng hai mutex, quan sát treo, dùng `gdb`/`thread apply all bt` để chẩn đoán, rồi sửa bằng thứ tự khóa.
7. Viết **thread pool** đơn giản: `N` luồng công nhân lấy "công việc" (con trỏ hàm + đối số) từ một hàng đợi có mutex/cond; có hàm `pool_submit` và `pool_shutdown`.
8. Viết chương trình "bữa tối của triết gia" 5 luồng, tránh deadlock bằng thứ tự khóa.
9. (Thử thách) Cài đặt bộ đếm tần suất từ song song: nhiều luồng đọc các file khác nhau, cập nhật một bảng băm chung; so sánh: một khóa toàn cục, khóa theo từng bucket, và bảng riêng mỗi luồng rồi gộp.

Mã nguồn mẫu: /code/chapter-15
