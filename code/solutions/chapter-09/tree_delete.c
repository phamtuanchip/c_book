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
