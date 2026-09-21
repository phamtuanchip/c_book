# Chương 10 — Struct, union, enum

## Mục tiêu chương

- Dùng `struct` để gom dữ liệu liên quan thành một kiểu mới; dùng `typedef` để đặt tên gọn.
- Hiểu **căn hàng (alignment)** và **đệm (padding)**, đo bằng `sizeof`/`offsetof`, và tối ưu bố cục.
- Dùng `union` cho các biến thể dữ liệu (tagged union) và đọc dữ liệu nhị phân.
- Dùng `enum` và bit-field đúng cách, biết giới hạn về tính di động.
- Xây dựng cấu trúc dữ liệu: danh sách liên kết, ngăn xếp (stack), hàng đợi (queue) vòng.
- Nắm cách khởi tạo, sao chép, truyền và trả về `struct`.

## 10.1. Vì sao cần struct?

Giả sử cần biểu diễn một sinh viên gồm tên, tuổi, điểm. Nếu chỉ có kiểu cơ bản, bạn sẽ có ba biến rời rạc, và với 100 sinh viên là ba mảng song song dễ lệch nhau:

```c
char names[100][50];
int ages[100];
double gpas[100];
```

**Struct (cấu trúc)** cho phép gom các dữ liệu có liên quan thành **một kiểu do bạn định nghĩa**:

```c
struct Student {
    char   name[50];
    int    age;
    double gpa;
};

struct Student students[100];      // 100 sinh viên, mỗi phần tử có đủ ba trường
```

## 10.2. Khai báo và sử dụng struct

### Khai báo

```c
struct Point {
    int x;
    int y;
};                 // NHỚ dấu ; sau dấu }
```

Đây chỉ là **bản thiết kế kiểu**; chưa có bộ nhớ nào được cấp cho đến khi bạn khai báo **biến**:

```c
struct Point p1;                          // biến chưa khởi tạo
struct Point p2 = {3, 4};                 // khởi tạo theo thứ tự trường
struct Point p3 = { .y = 8, .x = 2 };     // C99: khởi tạo theo tên (khuyến nghị: rõ ràng, an toàn khi đổi thứ tự)
struct Point p4 = {0};                    // tất cả trường bằng 0
```

### Truy cập thành viên

- **`.`** khi có **biến struct**: `p2.x`
- **`->`** khi có **con trỏ tới struct**: `pp->x` (viết tắt của `(*pp).x`)

```c
#include <stdio.h>

struct Point { int x, y; };

int main(void) {
    struct Point p = {3, 4};
    struct Point *pp = &p;

    p.x = 10;                    // sửa qua biến
    pp->y = 20;                  // sửa qua con trỏ
    printf("(%d, %d)\n", pp->x, (*pp).y);      // (10, 20)
    return 0;
}
```

### `typedef` để gọn hơn

```c
typedef struct Point {
    int x, y;
} Point;                         // giờ có thể viết "Point p;" thay vì "struct Point p;"

Point p = {1, 2};
```

Nếu struct tự tham chiếu tới chính nó (danh sách liên kết), phải giữ tên `struct Tên` bên trong vì `typedef` chưa có hiệu lực ở đó:

```c
typedef struct Node {
    int value;
    struct Node *next;           // dùng "struct Node", không phải "Node"
} Node;
```

### Sao chép, gán, truyền, trả về struct

Khác mảng, **struct có thể gán trực tiếp** và được **sao chép toàn bộ từng byte**:

```c
Point a = {1, 2};
Point b = a;                     // sao chép cả hai trường; b độc lập với a
b.x = 99;                        // a.x vẫn là 1
```

Khi **truyền vào hàm**, struct được sao chép (theo giá trị, giống mọi kiểu khác). Với struct lớn, truyền **con trỏ (`const`)** để tránh sao chép:

```c
double distance(Point a, Point b);            // sao chép 2 struct nhỏ: chấp nhận được
void   print_student(const struct Student *s);  // struct lớn: truyền con trỏ
```

Hàm có thể **trả về struct** (thuận tiện để trả nhiều giá trị):

```c
typedef struct { int quotient, remainder; } DivResult;

DivResult divide(int a, int b) {
    DivResult r = { a / b, a % b };
    return r;
}
```

**Lưu ý:** sao chép struct là **sao chép nông (shallow copy)**. Nếu struct chứa **con trỏ**, chỉ giá trị con trỏ được chép, không phải dữ liệu nó trỏ tới:

```c
typedef struct { char *name; } Person;

Person a = { strdup("An") };
Person b = a;                    // a.name và b.name cùng trỏ MỘT chuỗi
free(a.name);
// b.name giờ là con trỏ treo!
```

Muốn **sao chép sâu (deep copy)**, phải tự cấp phát và chép nội dung.

**So sánh:** không thể dùng `==` để so sánh hai struct. Hãy so sánh từng trường (đừng dùng `memcmp` trên struct vì byte đệm có thể khác nhau).

### Struct lồng nhau và mảng trong struct

```c
typedef struct {
    Point top_left;
    Point bottom_right;
} Rect;

Rect r = { .top_left = {0, 10}, .bottom_right = {5, 0} };
int width = r.bottom_right.x - r.top_left.x;
```

Mảng bên trong struct **được sao chép cùng struct** (khác với mảng truyền vào hàm):

```c
typedef struct { int data[3]; } Triple;
Triple t1 = {{1, 2, 3}};
Triple t2 = t1;                  // t2.data cũng là {1, 2, 3}, bản sao độc lập
```

## 10.3. Bố cục bộ nhớ: căn hàng và đệm

### Căn hàng (alignment)

CPU truy cập dữ liệu hiệu quả nhất khi địa chỉ của nó là **bội số** của kích thước (hoặc yêu cầu căn hàng) của kiểu. Ví dụ `int` (4 byte) thường phải đặt ở địa chỉ chia hết cho 4; `double` cho 8. Để đảm bảo điều đó, compiler **chèn các byte đệm (padding)** giữa các trường và cuối struct.

```c
struct A {
    char  c;      // 1 byte
    int   i;      // 4 byte, cần căn hàng 4
    char  d;      // 1 byte
};
```

Bố cục thực tế:

```text
offset:  0    1  2  3    4  5  6  7    8    9 10 11
       ┌────┬──────────┬───────────┬────┬──────────┐
       │ c  │ padding  │     i     │ d  │ padding  │
       └────┴──────────┴───────────┴────┴──────────┘
         1      3 byte      4 byte     1    3 byte      → tổng 12 byte
```

Padding ở cuối bảo đảm mảng `struct A arr[N]` mọi phần tử đều căn hàng đúng. Kích thước struct là bội số của yêu cầu căn hàng lớn nhất trong các trường.

### Sắp xếp lại trường để giảm kích thước

Xếp các trường theo **thứ tự giảm dần kích thước**:

```c
struct B {
    int   i;      // 4 byte, offset 0
    char  c;      // offset 4
    char  d;      // offset 5
};                // + 2 byte đệm cuối để tổng chia hết cho 4 -> 8 byte
```

`struct B` chỉ 8 byte so với 12 byte của `struct A`, dù chứa cùng dữ liệu. Với hàng triệu phần tử điều này đáng kể (giảm 33% bộ nhớ và cache tốt hơn).

Chương trình đo:

```c
// layout.c
#include <stdio.h>
#include <stddef.h>     // offsetof

struct A { char c; int i; char d; };
struct B { int i; char c; char d; };

int main(void) {
    printf("sizeof(struct A) = %zu\n", sizeof(struct A));    // 12
    printf("  offset c = %zu, i = %zu, d = %zu\n",
           offsetof(struct A, c), offsetof(struct A, i), offsetof(struct A, d));  // 0, 4, 8
    printf("sizeof(struct B) = %zu\n", sizeof(struct B));    // 8
    printf("  offset i = %zu, c = %zu, d = %zu\n",
           offsetof(struct B, i), offsetof(struct B, c), offsetof(struct B, d));  // 0, 4, 5
    return 0;
}
```

Macro `offsetof(type, member)` (trong `<stddef.h>`) cho **độ lệch byte** của trường so với đầu struct.

### Khi nào KHÔNG nên sắp xếp lại?

- **Định dạng file/giao thức mạng** có thứ tự trường cố định: **đừng** ghi struct thô ra file/socket (vì padding, endianness và kích thước kiểu khác nhau giữa các máy). Hãy **tuần tự hóa từng trường** theo thứ tự byte quy định.
- **Dễ đọc:** nhóm các trường liên quan lại với nhau có thể quan trọng hơn vài byte tiết kiệm.

Với việc ghi buộc phải đóng gói sát (ví dụ đọc header nhị phân), compiler có mở rộng `__attribute__((packed))` (gcc/clang) hoặc `#pragma pack(1)` để bỏ padding, nhưng truy cập trường không căn hàng có thể chậm hoặc lỗi trên vài CPU. Dùng thận trọng.

C11 cung cấp `_Alignof(type)` và `_Alignas(n)` để đọc/đặt yêu cầu căn hàng; `<stdalign.h>` định nghĩa `alignof` và `alignas`.

## 10.4. Union

**Union** giống struct nhưng **mọi trường dùng chung cùng một vùng nhớ**. Kích thước union bằng kích thước trường lớn nhất; tại một thời điểm chỉ một trường có ý nghĩa.

```c
union Value {
    int    i;
    float  f;
    char   bytes[4];
};
```

```text
       ┌──────────────────────┐
       │      4 byte chung    │
       └──────────────────────┘
        ↑  i, f và bytes[0..3] đều chồng lên vùng này
```

```c
union Value v;
v.i = 0x3F800000;           // ghi qua trường int
printf("%f\n", v.f);        // đọc lại qua float: 1.000000 (bit pattern của 1.0f)
printf("%zu\n", sizeof v);  // 4
```

Đọc một trường khác với trường vừa ghi được C99/C11 cho phép (cách này gọi là *type punning* qua union) nhưng kết quả phụ thuộc biểu diễn của kiểu; hãy dùng có chủ đích.

### Ứng dụng 1: tagged union (biến thể an toàn)

Kết hợp union với một **trường nhãn (tag)** cho biết trường nào đang có hiệu lực:

```c
typedef enum { VAL_INT, VAL_DOUBLE, VAL_STRING } ValueType;

typedef struct {
    ValueType type;
    union {
        long        i;
        double      d;
        const char *s;
    } as;
} Value;

void print_value(const Value *v) {
    switch (v->type) {
        case VAL_INT:    printf("int: %ld\n",    v->as.i); break;
        case VAL_DOUBLE: printf("double: %g\n",  v->as.d); break;
        case VAL_STRING: printf("string: %s\n",  v->as.s); break;
    }
}

int main(void) {
    Value a = { .type = VAL_INT,    .as.i = 42 };
    Value b = { .type = VAL_DOUBLE, .as.d = 3.14 };
    Value c = { .type = VAL_STRING, .as.s = "xin chao" };
    print_value(&a); print_value(&b); print_value(&c);
    return 0;
}
```

Mẫu này được dùng rộng rãi cho **trình thông dịch** (giá trị động), **cây cú pháp**, **sự kiện** với nhiều loại (chương 19).

### Ứng dụng 2: xem cùng dữ liệu theo nhiều cách

```c
typedef union {
    uint32_t value;
    struct { uint8_t b0, b1, b2, b3; } bytes;   // thấy từng byte (thứ tự phụ thuộc endianness)
} Word;

Word w = { .value = 0x12345678 };
printf("%02x\n", w.bytes.b0);      // 78 trên máy little-endian
```

### Cảnh báo về union

- Không có cơ chế nào ngăn bạn đọc sai trường — bạn phải tự quản lý bằng tag.
- Đọc trường khác kiểu có thể cho **giá trị bẫy hoặc không xác định** (ví dụ ghi `char[4]` rồi đọc `float` không đúng định dạng).

## 10.5. Enum

**Enum** định nghĩa tập hằng số nguyên có tên.

```c
enum Color { RED, GREEN, BLUE };          // RED = 0, GREEN = 1, BLUE = 2
enum Status { OK = 0, WARN = 10, ERR = 20 };   // giá trị tùy chọn
enum Level { LOW = 1, MID, HIGH };             // MID = 2, HIGH = 3 (tăng dần từ giá trị trước)

enum Color c = GREEN;
```

Trong C, hằng `enum` thực chất là **`int`** (không có kiểu mạnh như C++): `enum Color c = 42;` biên dịch được (có thể có cảnh báo). Tuy nhiên `enum` vẫn hữu ích vì:

1. **Đọc hiểu:** `state = STATE_CONNECTED` rõ hơn `state = 2`.
2. **Hằng lúc biên dịch:** dùng được làm nhãn `case`, kích thước mảng.
3. **Cảnh báo `switch`:** gcc `-Wall` cảnh báo nếu `switch` thiếu một giá trị `enum` (khi không có `default`).

```c
typedef enum {
    STATE_IDLE,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_CLOSED,
    STATE_COUNT               // mẹo: phần tử cuối cho biết số lượng
} State;

static const char *STATE_NAMES[STATE_COUNT] = {
    [STATE_IDLE]       = "idle",
    [STATE_CONNECTING] = "connecting",
    [STATE_CONNECTED]  = "connected",
    [STATE_CLOSED]     = "closed",
};

const char *state_name(State s) {
    return (s >= 0 && s < STATE_COUNT) ? STATE_NAMES[s] : "?";
}
```

Ví dụ `enum` như hằng nguyên thay cho `#define`:

```c
enum { MAX_NAME = 32, MAX_ITEMS = 100 };
char name[MAX_NAME];
```

### Enum dùng làm cờ bit

```c
enum Permission {
    PERM_READ  = 1 << 0,     // 001
    PERM_WRITE = 1 << 1,     // 010
    PERM_EXEC  = 1 << 2      // 100
};

unsigned perms = PERM_READ | PERM_WRITE;
if (perms & PERM_WRITE) { /* có quyền ghi */ }
perms &= ~PERM_WRITE;         // bỏ quyền ghi
```

## 10.6. Bit-field

Bit-field cho phép chỉ định số **bit** dành cho một trường, để đóng gói các cờ hoặc trường nhỏ:

```c
struct Flags {
    unsigned int ready   : 1;     // 1 bit
    unsigned int error   : 1;
    unsigned int mode    : 3;     // 3 bit: 0..7
    unsigned int reserved: 3;
};                                // tổng 8 bit, thường chiếm 4 byte (kích thước unit)

struct Flags f = {0};
f.ready = 1;
f.mode = 5;
printf("%u %u\n", f.ready, f.mode);
```

Hạn chế quan trọng: **thứ tự bit trong byte, cách đóng gói, và kích thước là do compiler quyết định** (implementation-defined) — nên **không đáng tin** để ánh xạ với phần cứng hay giao thức nhị phân giữa các compiler. Không lấy được địa chỉ (`&`) của bit-field. Với mục đích tiện lợi thì dùng được; với dữ liệu cần đúng từng bit, hãy dùng toán tử bit (`&`, `|`, `<<`) trên số nguyên có kích thước cố định.

## 10.7. Cấu trúc dữ liệu: danh sách liên kết đơn

**Danh sách liên kết (linked list)** là chuỗi các **nút** cấp phát riêng lẻ, mỗi nút giữ dữ liệu và con trỏ tới nút kế.

```text
head ──► [ 10 | ●─]──► [ 20 | ●─]──► [ 30 | NULL ]
```

So với mảng: chèn/xóa ở đầu là O(1) (không phải dịch phần tử), kích thước linh hoạt; nhưng truy cập theo chỉ số là O(n) và kém thân thiện với cache.

```c
// linked_list.c
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int value;
    struct Node *next;
} Node;

// Tạo nút mới; trả về NULL nếu hết bộ nhớ
static Node *node_create(int value) {
    Node *n = malloc(sizeof *n);
    if (!n) return NULL;
    n->value = value;
    n->next = NULL;
    return n;
}

// Chèn vào ĐẦU danh sách: O(1). *head được cập nhật => cần con trỏ tới con trỏ
int list_push_front(Node **head, int value) {
    Node *n = node_create(value);
    if (!n) return -1;
    n->next = *head;
    *head = n;
    return 0;
}

// Chèn vào CUỐI: O(n) vì phải đi tới cuối
int list_push_back(Node **head, int value) {
    Node *n = node_create(value);
    if (!n) return -1;
    if (*head == NULL) { *head = n; return 0; }
    Node *cur = *head;
    while (cur->next) cur = cur->next;
    cur->next = n;
    return 0;
}

// Tìm nút đầu tiên có giá trị value
Node *list_find(Node *head, int value) {
    for (Node *cur = head; cur; cur = cur->next)
        if (cur->value == value) return cur;
    return NULL;
}

// Xóa nút đầu tiên có giá trị value. Trả về 1 nếu xóa được, 0 nếu không thấy.
int list_remove(Node **head, int value) {
    for (Node **pp = head; *pp; pp = &(*pp)->next) {   // pp trỏ tới "ô chứa con trỏ tới nút hiện tại"
        if ((*pp)->value == value) {
            Node *dead = *pp;
            *pp = dead->next;                          // nối bỏ qua nút bị xóa
            free(dead);
            return 1;
        }
    }
    return 0;
}

// Đảo ngược danh sách tại chỗ: O(n)
void list_reverse(Node **head) {
    Node *prev = NULL, *cur = *head;
    while (cur) {
        Node *next = cur->next;    // nhớ nút kế trước khi đổi liên kết
        cur->next = prev;
        prev = cur;
        cur = next;
    }
    *head = prev;
}

void list_print(const Node *head) {
    for (const Node *cur = head; cur; cur = cur->next) printf("%d -> ", cur->value);
    printf("NULL\n");
}

// Giải phóng toàn bộ: nhớ lấy next TRƯỚC khi free
void list_free(Node *head) {
    while (head) {
        Node *next = head->next;
        free(head);
        head = next;
    }
}

int main(void) {
    Node *head = NULL;
    list_push_back(&head, 10);
    list_push_back(&head, 20);
    list_push_front(&head, 5);
    list_print(head);                 // 5 -> 10 -> 20 -> NULL

    list_remove(&head, 10);
    list_print(head);                 // 5 -> 20 -> NULL

    list_reverse(&head);
    list_print(head);                 // 20 -> 5 -> NULL

    printf("find 5: %s\n", list_find(head, 5) ? "co" : "khong");
    list_free(head);
    return 0;
}
```

### Giải thích kỹ thuật "con trỏ tới con trỏ" trong `list_remove`

Cách thông thường phải xử lý riêng trường hợp xóa nút **đầu** (vì phải cập nhật `head`) và trường hợp nút giữa/cuối (phải nhớ nút **trước**). Dùng `Node **pp` — con trỏ tới **ô chứa con trỏ** — cho phép xử lý cả hai giống hệt nhau, vì ô đó là `head` hoặc `prev->next`:

```text
head ──► [A] ──► [B] ──► [C]

pp = &head        → *pp là con trỏ tới A
pp = &A.next      → *pp là con trỏ tới B
Xóa B: *pp = B.next  (A.next giờ trỏ tới C)
```

Đây là kỹ thuật thanh lịch đáng nắm vững (tương tự Linus Torvalds từng nhắc đến như một ví dụ về "good taste" trong mã).

### Lỗi thường gặp với danh sách liên kết

- `free(head)` rồi mới đọc `head->next` (use-after-free) trong vòng lặp giải phóng.
- Quên cập nhật `head` khi chèn/xóa ở đầu (nếu truyền `Node *head` thay vì `Node **head`).
- Không xử lý danh sách rỗng (`head == NULL`).
- Tạo **vòng** vô tình (nút trỏ ngược) khiến các vòng lặp không kết thúc.

## 10.8. Ngăn xếp (stack) bằng mảng động

**Stack** là cấu trúc **vào sau, ra trước (LIFO)** với hai thao tác chính `push` và `pop`.

```c
// stack.c
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int    *data;
    size_t  size, cap;
} Stack;

int stack_init(Stack *s) {
    s->data = NULL; s->size = 0; s->cap = 0;
    return 0;
}

void stack_destroy(Stack *s) {
    free(s->data);
    s->data = NULL; s->size = s->cap = 0;
}

int stack_push(Stack *s, int v) {
    if (s->size == s->cap) {
        size_t nc = s->cap ? s->cap * 2 : 8;
        int *t = realloc(s->data, nc * sizeof *t);
        if (!t) return -1;
        s->data = t; s->cap = nc;
    }
    s->data[s->size++] = v;
    return 0;
}

// Trả về 0 nếu thành công, -1 nếu stack rỗng
int stack_pop(Stack *s, int *out) {
    if (s->size == 0) return -1;
    *out = s->data[--s->size];
    return 0;
}

int main(void) {
    Stack s;
    stack_init(&s);
    for (int i = 1; i <= 5; i++) stack_push(&s, i * 10);

    int v;
    while (stack_pop(&s, &v) == 0) printf("%d ", v);    // 50 40 30 20 10
    printf("\n");
    stack_destroy(&s);
    return 0;
}
```

Ứng dụng của stack: kiểm tra ngoặc cân bằng, chuyển biểu thức trung tố sang hậu tố, hoàn tác (undo), duyệt đồ thị theo chiều sâu, và bản thân **call stack** của chương trình.

## 10.9. Hàng đợi vòng (circular buffer queue)

**Queue** là cấu trúc **vào trước, ra trước (FIFO)**. Dùng mảng **vòng (ring buffer)** với hai chỉ số `head` (chỗ đọc) và `tail` (chỗ ghi), cả hai quay vòng bằng phép `% cap`.

```text
cap = 6, đã có 3 phần tử        thêm tiếp, tail quay vòng về đầu mảng
 index: 0  1  2  3  4  5          index: 0  1  2  3  4  5
       [ ][ ][A][B][C][ ]               [F][ ][A][B][C][D]  ...
             ↑head    ↑tail                   ↑head          tail = 1
```

```c
// queue.c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int    *buf;
    size_t  cap;
    size_t  head;      // vị trí phần tử sẽ lấy ra
    size_t  count;     // số phần tử hiện có (dùng count để phân biệt đầy/rỗng)
} Queue;

bool queue_init(Queue *q, size_t cap) {
    q->buf = malloc(cap * sizeof *q->buf);
    if (!q->buf) return false;
    q->cap = cap; q->head = 0; q->count = 0;
    return true;
}

void queue_destroy(Queue *q) { free(q->buf); q->buf = NULL; }

bool queue_push(Queue *q, int v) {
    if (q->count == q->cap) return false;                    // đầy
    size_t tail = (q->head + q->count) % q->cap;
    q->buf[tail] = v;
    q->count++;
    return true;
}

bool queue_pop(Queue *q, int *out) {
    if (q->count == 0) return false;                          // rỗng
    *out = q->buf[q->head];
    q->head = (q->head + 1) % q->cap;
    q->count--;
    return true;
}

int main(void) {
    Queue q;
    if (!queue_init(&q, 4)) return 1;

    for (int i = 1; i <= 6; i++) {
        if (!queue_push(&q, i)) printf("queue day, bo qua %d\n", i);
    }
    int v;
    while (queue_pop(&q, &v)) printf("%d ", v);               // 1 2 3 4
    printf("\n");

    queue_destroy(&q);
    return 0;
}
```

Cách dùng `count` đơn giản hơn so với cách dùng hai chỉ số và phải "hy sinh" một ô để phân biệt đầy với rỗng. Queue vòng cũng là nền tảng của bộ đệm giữa các luồng (chương 15) và giữa thiết bị-driver.

## 10.10. Thiết kế kiểu dữ liệu trừu tượng (ADT) — che giấu chi tiết

Với thư viện, bạn nên giấu cấu trúc bên trong để người dùng không phụ thuộc vào chi tiết. Kỹ thuật **opaque pointer**:

```c
// stack.h — giao diện công khai
typedef struct Stack Stack;               // khai báo kiểu, KHÔNG lộ trường

Stack *stack_create(void);
void   stack_free(Stack *s);
int    stack_push(Stack *s, int v);
int    stack_pop(Stack *s, int *out);
```

```c
// stack.c — cài đặt riêng tư
#include "stack.h"
#include <stdlib.h>

struct Stack {                            // định nghĩa đầy đủ chỉ nằm trong .c
    int *data;
    size_t size, cap;
};
/* ... cài đặt ... */
```

Người dùng chỉ có `Stack *` — không đọc/sửa trực tiếp `size`, `cap` được; bạn có thể đổi cài đặt bên trong mà không làm hỏng mã người dùng. Đây là cách hiện thực **đóng gói (encapsulation)** trong C.

## 10.11. Lỗi thường gặp

| Lỗi | Hậu quả | Cách tránh |
|---|---|---|
| Thiếu `;` sau `}` của struct | Lỗi biên dịch khó hiểu | Nhớ `};` |
| Dùng `.` thay `->` (hoặc ngược lại) | Lỗi biên dịch | `.` cho biến, `->` cho con trỏ |
| Ghi struct thô ra file/mạng | Không tương thích giữa máy | Tuần tự hóa từng trường |
| So sánh struct bằng `==` hoặc `memcmp` | Không biên dịch / sai do padding | So sánh từng trường |
| Sao chép nông struct có con trỏ | Double free / dangling | Sao chép sâu hoặc quy định ownership |
| Không khởi tạo struct cục bộ | Trường chứa rác | `= {0}` |
| Đọc sai trường union | Giá trị vô nghĩa | Dùng tagged union |
| Tin bit-field cho giao thức nhị phân | Không di động | Dùng toán tử bit |
| Quên `free` từng nút hoặc `free` sai thứ tự | Rò rỉ / use-after-free | Lấy `next` trước khi `free` |

## 10.12. Tóm tắt

- `struct` gom dữ liệu; `.` cho biến, `->` cho con trỏ; struct được sao chép **nông** khi gán/truyền.
- Compiler chèn padding để căn hàng; sắp xếp trường từ lớn xuống nhỏ để tiết kiệm; đo bằng `sizeof` và `offsetof`.
- `union` dùng chung vùng nhớ; luôn kèm **tag** khi tạo biến thể.
- `enum` là hằng số nguyên có tên, tiện cho trạng thái và cờ; bit-field không di động.
- Cấu trúc dữ liệu tự làm (danh sách, stack, queue) quy về việc quản lý con trỏ và bộ nhớ đúng; luôn viết cặp tạo/hủy và kiểm thử bằng sanitizer.

## 10.13. Bài tập

1. Định nghĩa `struct Student { char name[50]; int age; double gpa; }`. Viết chương trình nhập `n` sinh viên (mảng cấp phát động), sắp xếp theo `gpa` giảm dần bằng `qsort`, và in bảng.
2. Sắp xếp lại các trường của `struct { char a; double b; char c; int d; short e; }` để có kích thước nhỏ nhất. Kiểm chứng bằng `sizeof` và `offsetof`, giải thích.
3. Cài đặt tagged union `Value` (int/double/string) kèm hàm `value_equals` và `value_print`.
4. Mở rộng danh sách liên kết: `list_insert_sorted`, `list_length`, `list_nth`, `list_concat`. Kiểm thử bằng AddressSanitizer.
5. Cài đặt **danh sách liên kết đôi** (`prev` và `next`) có hàm `splice` chuyển một đoạn nút sang danh sách khác.
6. Dùng `Stack` để viết chương trình kiểm tra chuỗi ngoặc `()[]{}` cân bằng.
7. Dùng `Queue` để mô phỏng phục vụ khách hàng: mỗi khách có id và thời gian phục vụ.
8. Viết chương trình đọc một file nhị phân có header (magic 4 byte, phiên bản `uint16_t`, số bản ghi `uint32_t`) bằng cách đọc **từng trường** và giải thích vì sao không đọc thẳng vào struct.
9. (Thử thách) Cài đặt bảng băm (hash table) xử lý va chạm bằng danh sách liên kết cho ánh xạ chuỗi → số nguyên, với `ht_put`, `ht_get`, `ht_remove`, `ht_free`.

Mã nguồn mẫu: /code/chapter-10
