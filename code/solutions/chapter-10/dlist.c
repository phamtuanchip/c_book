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
