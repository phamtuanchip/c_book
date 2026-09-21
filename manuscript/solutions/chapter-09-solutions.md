# Chương 9 — Lời giải bài tập

## Bài 1: rò rỉ với Valgrind và ASan

Chạy `leak_demo.c` (chương 9):

```bash
gcc -std=c11 -g -fsanitize=address -o leak_demo leak_demo.c && ./leak_demo
# ==...==ERROR: LeakSanitizer: detected memory leaks
# Direct leak of 1024 byte(s) in 1 object(s) allocated from:
#     #1 ... in process leak_demo.c:7
# SUMMARY: AddressSanitizer: 1024000 byte(s) leaked in 1000 allocation(s).
```

`process` cấp phát `buf` mỗi lần gọi mà không `free`. Sửa: thêm `free(buf);` trước khi hàm kết thúc (mọi đường thoát). Chạy lại: báo cáo sạch.

## Bài 2: tái hiện ba lỗi và thông báo của ASan

```c
// heap_bugs.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    int which = argc > 1 ? atoi(argv[1]) : 0;
    if (which == 1) {                       /* use-after-free */
        int *p = malloc(sizeof *p);
        *p = 1;
        free(p);
        printf("%d\n", *p);
    } else if (which == 2) {                /* double free */
        char *s = malloc(10);
        free(s);
        free(s);
    } else if (which == 3) {                /* heap overflow off-by-one: quên chỗ cho '\0' */
        char *s = malloc(5);
        strcpy(s, "hello");
        free(s);
    } else {
        puts("dung: ./heap_bugs 1|2|3");
    }
    return 0;
}
```

| Tham số | Thông báo ASan chính |
|---|---|
| `1` | `heap-use-after-free` (READ of size 4) |
| `2` | `attempting double-free` |
| `3` | `heap-buffer-overflow` (WRITE of size 6 — 5 ký tự + `'\0'` vào khối 5 byte) |

## Bài 3: `str_dup` và `str_concat`

```c
// str_utils.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Trả về bản sao mới của s. NGƯỜI GỌI phải free(). NULL nếu s == NULL hoặc hết bộ nhớ. */
static char *str_dup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;                  // +1 cho '\0'
    char *p = malloc(n);
    if (!p) return NULL;
    memcpy(p, s, n);
    return p;
}

/* Trả về chuỗi mới a+b. NGƯỜI GỌI phải free(). Kiểm tra tràn size_t khi cộng độ dài. */
static char *str_concat(const char *a, const char *b) {
    if (!a || !b) return NULL;
    size_t la = strlen(a), lb = strlen(b);
    if (la > (size_t)-1 - lb - 1) return NULL;
    char *p = malloc(la + lb + 1);
    if (!p) return NULL;
    memcpy(p, a, la);
    memcpy(p + la, b, lb + 1);                 // chép cả '\0'
    return p;
}

int main(void) {
    char *a = str_dup("xin ");
    char *c = str_concat(a, "chao");
    if (c) puts(c);
    free(a);
    free(c);
    return 0;
}
```

## Bài 4: mở rộng `Vec`

Thêm vào `vec.c` (chương 9):

```c
// vec_ext.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { int *data; size_t size, cap; } Vec;

static void vec_init(Vec *v) { v->data = NULL; v->size = v->cap = 0; }
static void vec_destroy(Vec *v) { free(v->data); vec_init(v); }

static int vec_reserve(Vec *v, size_t need) {
    if (need <= v->cap) return 0;
    size_t nc = v->cap ? v->cap : 4;
    while (nc < need) nc *= 2;
    int *t = realloc(v->data, nc * sizeof *t);
    if (!t) return -1;
    v->data = t; v->cap = nc;
    return 0;
}

static int vec_push(Vec *v, int x) {
    if (vec_reserve(v, v->size + 1) != 0) return -1;
    v->data[v->size++] = x;
    return 0;
}

/* Trả 0 nếu lấy được, -1 nếu rỗng */
static int vec_pop(Vec *v, int *out) {
    if (v->size == 0) return -1;
    *out = v->data[--v->size];
    return 0;
}

static int vec_get(const Vec *v, size_t i, int *out) {
    if (i >= v->size) return -1;
    *out = v->data[i];
    return 0;
}

/* Chèn x vào vị trí index (0..size): dịch các phần tử sau sang phải */
static int vec_insert(Vec *v, size_t index, int x) {
    if (index > v->size) return -1;
    if (vec_reserve(v, v->size + 1) != 0) return -1;
    memmove(v->data + index + 1, v->data + index, (v->size - index) * sizeof *v->data);
    v->data[index] = x;
    v->size++;
    return 0;
}

static int vec_remove(Vec *v, size_t index) {
    if (index >= v->size) return -1;
    memmove(v->data + index, v->data + index + 1, (v->size - index - 1) * sizeof *v->data);
    v->size--;
    return 0;
}

int main(void) {
    Vec v;
    vec_init(&v);
    for (int i = 0; i < 5; i++) vec_push(&v, i * 10);        // 0 10 20 30 40
    vec_insert(&v, 2, 99);                                    // 0 10 99 20 30 40
    vec_remove(&v, 0);                                        // 10 99 20 30 40
    int x;
    vec_pop(&v, &x);                                          // x = 40
    for (size_t i = 0; i < v.size; i++) { vec_get(&v, i, &x); printf("%d ", x); }
    printf("\n");
    vec_destroy(&v);
    return 0;
}
```

Dùng `memmove` (không phải `memcpy`) vì vùng nguồn và đích **chồng lấn**.

## Bài 5: `tree_delete` và `tree_height`

```c
// tree_delete.c
#include <stdio.h>
#include <stdlib.h>

typedef struct Node { int key; struct Node *left, *right; } Node;

static Node *node_new(int key) {
    Node *n = calloc(1, sizeof *n);
    if (n) n->key = key;
    return n;
}

static Node *insert(Node *r, int key) {
    if (!r) return node_new(key);
    if (key < r->key) r->left = insert(r->left, key);
    else if (key > r->key) r->right = insert(r->right, key);
    return r;
}

static int tree_height(const Node *r) {                    // cây rỗng có chiều cao 0
    if (!r) return 0;
    int hl = tree_height(r->left), hr = tree_height(r->right);
    return 1 + (hl > hr ? hl : hr);
}

static Node *min_node(Node *r) { while (r->left) r = r->left; return r; }

/* Xóa key khỏi cây; trả về gốc mới. Ba trường hợp: lá, một con, hai con. */
static Node *tree_delete(Node *r, int key) {
    if (!r) return NULL;
    if (key < r->key) { r->left = tree_delete(r->left, key); return r; }
    if (key > r->key) { r->right = tree_delete(r->right, key); return r; }

    if (!r->left)  { Node *c = r->right; free(r); return c; }   // 0 hoặc 1 con (phải)
    if (!r->right) { Node *c = r->left;  free(r); return c; }   // 1 con (trái)

    Node *s = min_node(r->right);                  // hai con: thay bằng kế nhiệm (nhỏ nhất bên phải)
    r->key = s->key;
    r->right = tree_delete(r->right, s->key);
    return r;
}

static void tree_free(Node *r) { if (!r) return; tree_free(r->left); tree_free(r->right); free(r); }
static void inorder(const Node *r) { if (!r) return; inorder(r->left); printf("%d ", r->key); inorder(r->right); }

int main(void) {
    Node *root = NULL;
    int keys[] = {50, 30, 70, 20, 40, 60, 80};
    for (size_t i = 0; i < sizeof keys / sizeof keys[0]; i++) root = insert(root, keys[i]);
    root = tree_delete(root, 30);                  // nút có hai con
    root = tree_delete(root, 20);                  // lá
    inorder(root);                                 // 40 50 60 70 80
    printf("\nchieu cao = %d\n", tree_height(root));
    tree_free(root);
    return 0;
}
```

Chạy với `-fsanitize=address` để chắc chắn mọi nút bị xóa đều được `free` và không có use-after-free.

## Bài 6: đọc cả file vào bộ đệm động

```c
// slurp.c
#include <stdio.h>
#include <stdlib.h>

/* Đọc toàn bộ f vào bộ đệm tăng dần. Trả bộ đệm (kèm '\0'), *len là số byte; NULL nếu lỗi. */
static char *slurp(FILE *f, size_t *len) {
    size_t cap = 4096, n = 0;
    char *buf = malloc(cap);
    if (!buf) return NULL;
    size_t got;
    while ((got = fread(buf + n, 1, cap - n - 1, f)) > 0) {
        n += got;
        if (cap - n < 2) {                          // sắp đầy: nhân đôi
            char *t = realloc(buf, cap * 2);
            if (!t) { free(buf); return NULL; }
            buf = t;
            cap *= 2;
        }
    }
    if (ferror(f)) { free(buf); return NULL; }
    buf[n] = '\0';
    if (len) *len = n;
    return buf;
}

int main(int argc, char **argv) {
    FILE *f = argc > 1 ? fopen(argv[1], "rb") : stdin;
    if (!f) { perror("fopen"); return 1; }
    size_t n;
    char *data = slurp(f, &n);
    if (!data) { fprintf(stderr, "loi doc\n"); return 1; }
    long lines = 0;
    for (size_t i = 0; i < n; i++) if (data[i] == '\n') lines++;
    printf("%zu byte, %ld dong\n", n, lines);
    free(data);
    if (f != stdin) fclose(f);
    return 0;
}
```

## Bài 7: bộ cấp phát arena

```c
// arena.c
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct { unsigned char *base; size_t cap, used; } Arena;

static int arena_init(Arena *a, size_t cap) {
    a->base = malloc(cap);
    a->cap = cap;
    a->used = 0;
    return a->base ? 0 : -1;
}

/* Cấp n byte, căn hàng 16; NULL nếu hết chỗ. Không có free từng khối. */
static void *arena_alloc(Arena *a, size_t n) {
    size_t start = (a->used + 15u) & ~(size_t)15u;
    if (start > a->cap || n > a->cap - start) return NULL;
    a->used = start + n;
    return a->base + start;
}

static void arena_free_all(Arena *a) { free(a->base); a->base = NULL; a->cap = a->used = 0; }

int main(void) {
    Arena a;
    if (arena_init(&a, 1024) != 0) return 1;
    int *xs = arena_alloc(&a, 10 * sizeof *xs);
    char *name = arena_alloc(&a, 32);
    if (!xs || !name) return 1;
    xs[0] = 42;
    snprintf(name, 32, "arena");
    printf("%d %s used=%zu\n", xs[0], name, a.used);
    arena_free_all(&a);                            // trả tất cả một lần
    return 0;
}
```

Arena hữu ích khi nhiều đối tượng cùng vòng đời (ví dụ AST của một lần phân tích, dữ liệu của một yêu cầu HTTP): cấp phát rất nhanh, không phân mảnh, giải phóng một lần, không thể quên `free` từng cái.

## Bài 8: `tracked_malloc` / `tracked_free`

```c
// tracked_alloc.c
#include <stdio.h>
#include <stdlib.h>

static size_t g_allocs, g_frees, g_bytes;

static void report(void) {
    printf("cap phat: %zu, giai phong: %zu, tong %zu byte%s\n",
           g_allocs, g_frees, g_bytes, g_allocs != g_frees ? "  <-- CO RO RI!" : "");
}

static void *tracked_malloc(size_t n) {
    static int registered;
    if (!registered) { atexit(report); registered = 1; }       // in báo cáo khi thoát
    void *p = malloc(n);
    if (p) { g_allocs++; g_bytes += n; }
    return p;
}

static void tracked_free(void *p) {
    if (p) g_frees++;
    free(p);
}

int main(void) {
    int *a = tracked_malloc(40);
    int *b = tracked_malloc(80);
    tracked_free(a);
    (void)b;                                    // cố ý không free b để thấy cảnh báo
    return 0;
}
```

Đây là cách "tự chế" đơn giản; không cho biết **dòng nào** rò rỉ. Muốn biết, mở rộng bằng macro `#define MALLOC(n) tracked_malloc((n), __FILE__, __LINE__)` và lưu danh sách các khối còn sống — về bản chất là cách công cụ như Valgrind và LeakSanitizer làm việc.
