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
