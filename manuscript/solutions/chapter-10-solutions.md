# Chương 10 — Lời giải bài tập

## Bài 1: danh sách sinh viên, sắp xếp theo `gpa` giảm dần

```c
// students.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Student {
    char   name[50];
    int    age;
    double gpa;
};

static int cmp_gpa_desc(const void *a, const void *b) {
    const struct Student *x = a, *y = b;
    return (x->gpa < y->gpa) - (x->gpa > y->gpa);
}

int main(void) {
    int n;
    printf("So sinh vien: ");
    if (scanf("%d", &n) != 1 || n <= 0) return 1;

    struct Student *s = calloc((size_t)n, sizeof *s);
    if (!s) return 1;
    for (int i = 0; i < n; i++) {
        printf("Ten tuoi gpa #%d: ", i + 1);
        if (scanf("%49s %d %lf", s[i].name, &s[i].age, &s[i].gpa) != 3) { free(s); return 1; }
    }
    qsort(s, (size_t)n, sizeof *s, cmp_gpa_desc);

    printf("\n%-20s %4s %5s\n", "Ten", "Tuoi", "GPA");
    for (int i = 0; i < n; i++) printf("%-20s %4d %5.2f\n", s[i].name, s[i].age, s[i].gpa);
    free(s);
    return 0;
}
```

Chú ý: `%49s` giới hạn độ dài (mảng 50), và hàm so sánh trả `(a<b) - (a>b)` thay vì `a - b` để tránh sai số/tràn.

## Bài 2: sắp xếp lại trường để giảm kích thước

```c
// reorder.c
#include <stddef.h>
#include <stdio.h>

struct Bad  { char a; double b; char c; int d; short e; };    // đệm nhiều
struct Good { double b; int d; short e; char a; char c; };    // giảm dần theo kích thước

int main(void) {
    printf("Bad  = %zu byte\n", sizeof(struct Bad));           // 32
    printf("Good = %zu byte\n", sizeof(struct Good));          // 16
    return 0;
}
```

Giải thích (x86-64): `Bad`: `a` (offset 0) + 7 byte đệm để `b` căn hàng 8 (offset 8); `c` ở 16 + 3 đệm; `d` ở 20; `e` ở 24 + 6 đệm cuối để tổng chia hết cho 8 → **32 byte**. `Good`: `b` 0–7, `d` 8–11, `e` 12–13, `a` 14, `c` 15 → vừa đúng **16 byte**, không đệm.

## Bài 3: tagged union `Value`

```c
// value.c
#include <stdio.h>
#include <string.h>

typedef enum { VAL_INT, VAL_DOUBLE, VAL_STRING } ValueType;

typedef struct {
    ValueType type;
    union { long i; double d; const char *s; } as;
} Value;

static int value_equals(const Value *a, const Value *b) {
    if (a->type != b->type) return 0;
    switch (a->type) {
        case VAL_INT:    return a->as.i == b->as.i;
        case VAL_DOUBLE: return a->as.d == b->as.d;
        case VAL_STRING: return strcmp(a->as.s, b->as.s) == 0;
    }
    return 0;
}

static void value_print(const Value *v) {
    switch (v->type) {
        case VAL_INT:    printf("int(%ld)", v->as.i); break;
        case VAL_DOUBLE: printf("double(%g)", v->as.d); break;
        case VAL_STRING: printf("string(\"%s\")", v->as.s); break;
    }
}

int main(void) {
    Value a = { .type = VAL_INT, .as.i = 42 }, b = { .type = VAL_INT, .as.i = 42 };
    Value c = { .type = VAL_STRING, .as.s = "xin chao" };
    value_print(&a); printf(" == "); value_print(&b); printf(" ? %d\n", value_equals(&a, &b));   // 1
    value_print(&a); printf(" == "); value_print(&c); printf(" ? %d\n", value_equals(&a, &c));   // 0
    return 0;
}
```

Luôn kiểm tra **tag** trước khi đọc trường union — đó là điều biến union thành kiểu an toàn.

## Bài 4: mở rộng danh sách liên kết

```c
// list_ext.c
#include <stdio.h>
#include <stdlib.h>

typedef struct Node { int value; struct Node *next; } Node;

static Node *node_new(int v) { Node *n = malloc(sizeof *n); if (n) { n->value = v; n->next = NULL; } return n; }

/* Chèn vào danh sách đã sắp xếp tăng dần, giữ thứ tự. */
static int list_insert_sorted(Node **head, int v) {
    Node *n = node_new(v);
    if (!n) return -1;
    Node **pp = head;
    while (*pp && (*pp)->value < v) pp = &(*pp)->next;     // dừng ở chỗ nút đầu tiên >= v
    n->next = *pp;
    *pp = n;
    return 0;
}

static size_t list_length(const Node *h) { size_t c = 0; for (; h; h = h->next) c++; return c; }

/* Trả nút thứ i (0-based) hoặc NULL */
static Node *list_nth(Node *h, size_t i) { while (h && i--) h = h->next; return h; }

/* Nối b vào cuối a; trả về đầu danh sách kết quả (b thuộc về danh sách mới, đừng free hai lần) */
static Node *list_concat(Node *a, Node *b) {
    if (!a) return b;
    Node *t = a;
    while (t->next) t = t->next;
    t->next = b;
    return a;
}

static void list_free(Node *h) { while (h) { Node *n = h->next; free(h); h = n; } }

int main(void) {
    Node *a = NULL, *b = NULL;
    int va[] = {30, 10, 20}, vb[] = {5, 25};
    for (int i = 0; i < 3; i++) list_insert_sorted(&a, va[i]);      // 10 20 30
    for (int i = 0; i < 2; i++) list_insert_sorted(&b, vb[i]);      // 5 25
    a = list_concat(a, b);                                          // 10 20 30 5 25
    for (Node *p = a; p; p = p->next) printf("%d ", p->value);
    printf("\nlen = %zu, nth(3) = %d\n", list_length(a), list_nth(a, 3)->value);   // 5, 5
    list_free(a);                                                   // giải phóng cả hai (b đã nằm trong a)
    return 0;
}
```

## Bài 5: danh sách liên kết đôi có `splice`

```c
// dlist.c
#include <stdio.h>
#include <stdlib.h>

typedef struct DNode { int v; struct DNode *prev, *next; } DNode;
typedef struct { DNode *head, *tail; } DList;

static int push_back(DList *l, int v) {
    DNode *n = malloc(sizeof *n);
    if (!n) return -1;
    n->v = v; n->next = NULL; n->prev = l->tail;
    if (l->tail) l->tail->next = n; else l->head = n;
    l->tail = n;
    return 0;
}

/* Cắt đoạn [first..last] (cùng thuộc `from`) và chèn vào `to` ngay sau nút `pos` (NULL = chèn ở đầu). O(1). */
static void splice(DList *from, DNode *first, DNode *last, DList *to, DNode *pos) {
    /* 1) gỡ đoạn khỏi from */
    if (first->prev) first->prev->next = last->next; else from->head = last->next;
    if (last->next)  last->next->prev = first->prev; else from->tail = first->prev;

    /* 2) gắn vào to sau pos */
    DNode *after = pos ? pos->next : to->head;
    first->prev = pos;
    last->next = after;
    if (pos) pos->next = first; else to->head = first;
    if (after) after->prev = last; else to->tail = last;
}

static void print(const DList *l) { for (DNode *p = l->head; p; p = p->next) printf("%d ", p->v); printf("\n"); }
static void free_list(DList *l) { for (DNode *p = l->head, *n; p; p = n) { n = p->next; free(p); } l->head = l->tail = NULL; }

int main(void) {
    DList a = {0}, b = {0};
    for (int i = 1; i <= 5; i++) push_back(&a, i);           // 1 2 3 4 5
    push_back(&b, 100); push_back(&b, 200);                  // 100 200
    splice(&a, a.head->next, a.head->next->next, &b, b.head);   // chuyển [2,3] vào b sau 100
    print(&a);                                               // 1 4 5
    print(&b);                                               // 100 2 3 200
    free_list(&a); free_list(&b);
    return 0;
}
```

Ưu điểm của danh sách đôi: xóa/chèn một nút đã biết địa chỉ là **O(1)** và `splice` chuyển cả đoạn không cần sao chép.

## Bài 6: kiểm tra ngoặc cân bằng bằng stack

```c
// brackets.c
#include <stdio.h>
#include <string.h>

static int balanced(const char *s) {
    char stack[256];
    size_t top = 0;
    for (; *s; s++) {
        char c = *s;
        if (c == '(' || c == '[' || c == '{') {
            if (top == sizeof stack) return 0;                 // quá sâu
            stack[top++] = c;
        } else if (c == ')' || c == ']' || c == '}') {
            if (top == 0) return 0;                            // đóng khi chưa mở
            char o = stack[--top];
            if ((c == ')' && o != '(') || (c == ']' && o != '[') || (c == '}' && o != '{')) return 0;
        }
    }
    return top == 0;                                           // không còn ngoặc mở dở
}

int main(void) {
    const char *tests[] = {"([]{})", "([)]", "((", "", "a(b)c"};
    for (size_t i = 0; i < sizeof tests / sizeof tests[0]; i++)
        printf("%-8s -> %s\n", tests[i], balanced(tests[i]) ? "can bang" : "sai");
    return 0;
}
```

## Bài 7: mô phỏng phục vụ khách bằng hàng đợi

```c
// bank.c
#include <stdio.h>

typedef struct { int id, service; } Customer;

#define CAP 16
typedef struct { Customer buf[CAP]; int head, count; } Queue;

static int push(Queue *q, Customer c) {
    if (q->count == CAP) return -1;
    q->buf[(q->head + q->count++) % CAP] = c;
    return 0;
}
static int pop(Queue *q, Customer *c) {
    if (!q->count) return -1;
    *c = q->buf[q->head];
    q->head = (q->head + 1) % CAP;
    q->count--;
    return 0;
}

int main(void) {
    Queue q = {0};
    int service[] = {3, 1, 4, 2};
    for (int i = 0; i < 4; i++) push(&q, (Customer){ i + 1, service[i] });

    int clock = 0;
    Customer c;
    while (pop(&q, &c) == 0) {
        printf("t=%2d: bat dau phuc vu khach %d (%d phut)\n", clock, c.id, c.service);
        clock += c.service;
    }
    printf("t=%2d: xong\n", clock);                            // 10
    return 0;
}
```

## Bài 8: đọc header nhị phân từng trường

Không nên `fread(&hdr, sizeof hdr, 1, f)` vì: (1) **padding** giữa các trường (`uint16_t` sau 4 byte magic, rồi `uint32_t` cần căn hàng 4 → 2 byte đệm không có trong file); (2) **endianness** của file có thể khác của máy; (3) `sizeof` struct do compiler quyết định. Đọc từng trường với thứ tự byte cố định:

```c
// read_header.c
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int read_u16_le(FILE *f, uint16_t *v) {
    unsigned char b[2];
    if (fread(b, 1, 2, f) != 2) return -1;
    *v = (uint16_t)(b[0] | (b[1] << 8));
    return 0;
}
static int read_u32_le(FILE *f, uint32_t *v) {
    unsigned char b[4];
    if (fread(b, 1, 4, f) != 4) return -1;
    *v = (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "cach dung: %s file\n", argv[0]); return 2; }
    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror(argv[1]); return 1; }

    char magic[4];
    uint16_t version;
    uint32_t count;
    if (fread(magic, 1, 4, f) != 4 || memcmp(magic, "CBK1", 4) != 0 ||
        read_u16_le(f, &version) != 0 || read_u32_le(f, &count) != 0) {
        fprintf(stderr, "header khong hop le\n");
        fclose(f);
        return 1;
    }
    printf("phien ban %u, %u ban ghi\n", (unsigned)version, (unsigned)count);
    fclose(f);
    return 0;
}
```

(Định dạng giả định: magic `CBK1`, phiên bản và số bản ghi theo little-endian.) Nhớ kiểm tra `count` không vượt quá kích thước file trước khi cấp phát theo nó.

## Bài 9: bảng băm (chaining)

```c
// hashtable.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Entry { char *key; int value; struct Entry *next; } Entry;
typedef struct { Entry **buckets; size_t nbuckets; } HashTable;

static unsigned long hash_str(const char *s) {                 // djb2
    unsigned long h = 5381;
    for (; *s; s++) h = h * 33 + (unsigned char)*s;
    return h;
}

static HashTable *ht_new(size_t n) {
    HashTable *t = malloc(sizeof *t);
    if (!t) return NULL;
    t->buckets = calloc(n, sizeof *t->buckets);
    if (!t->buckets) { free(t); return NULL; }
    t->nbuckets = n;
    return t;
}

static int ht_put(HashTable *t, const char *key, int value) {
    size_t i = hash_str(key) % t->nbuckets;
    for (Entry *e = t->buckets[i]; e; e = e->next)
        if (strcmp(e->key, key) == 0) { e->value = value; return 0; }   // cập nhật nếu đã có
    Entry *e = malloc(sizeof *e);
    if (!e) return -1;
    size_t len = strlen(key) + 1;
    e->key = malloc(len);
    if (!e->key) { free(e); return -1; }
    memcpy(e->key, key, len);
    e->value = value;
    e->next = t->buckets[i];
    t->buckets[i] = e;
    return 0;
}

static int ht_get(const HashTable *t, const char *key, int *out) {
    for (Entry *e = t->buckets[hash_str(key) % t->nbuckets]; e; e = e->next)
        if (strcmp(e->key, key) == 0) { *out = e->value; return 0; }
    return -1;
}

static int ht_remove(HashTable *t, const char *key) {
    for (Entry **pp = &t->buckets[hash_str(key) % t->nbuckets]; *pp; pp = &(*pp)->next) {
        if (strcmp((*pp)->key, key) == 0) {
            Entry *dead = *pp;
            *pp = dead->next;
            free(dead->key);
            free(dead);
            return 0;
        }
    }
    return -1;
}

static void ht_free(HashTable *t) {
    if (!t) return;
    for (size_t i = 0; i < t->nbuckets; i++)
        for (Entry *e = t->buckets[i], *n; e; e = n) { n = e->next; free(e->key); free(e); }
    free(t->buckets);
    free(t);
}

int main(void) {
    HashTable *t = ht_new(16);
    if (!t) return 1;
    ht_put(t, "an", 10); ht_put(t, "binh", 20); ht_put(t, "an", 11);   // cập nhật "an"
    int v;
    if (ht_get(t, "an", &v) == 0) printf("an = %d\n", v);              // 11
    ht_remove(t, "binh");
    printf("binh %s\n", ht_get(t, "binh", &v) == 0 ? "con" : "da xoa");
    ht_free(t);
    return 0;
}
```

Độ phức tạp trung bình `O(1)` nếu hàm băm phân bố tốt và hệ số tải (`số phần tử / nbuckets`) được giữ thấp — bài mở rộng: tự động tăng số bucket khi hệ số tải > 0,75 (rehash).
