// tree.c
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int key;
    struct Node *left, *right;
} Node;

static Node *node_new(int key) {
    Node *n = malloc(sizeof *n);
    if (!n) return NULL;
    n->key = key;
    n->left = n->right = NULL;
    return n;
}

// Chèn khóa vào cây; trả về gốc mới (có thể là cùng gốc cũ). Trả NULL nếu hết bộ nhớ ở gốc.
Node *tree_insert(Node *root, int key) {
    if (root == NULL) return node_new(key);
    if (key < root->key) {
        Node *r = tree_insert(root->left, key);
        if (r == NULL) return root;                 // hết bộ nhớ: bỏ qua, giữ nguyên cây
        root->left = r;
    } else if (key > root->key) {
        Node *r = tree_insert(root->right, key);
        if (r == NULL) return root;
        root->right = r;
    }                                                // key trùng: không làm gì
    return root;
}

int tree_contains(const Node *root, int key) {
    while (root) {
        if (key == root->key) return 1;
        root = key < root->key ? root->left : root->right;
    }
    return 0;
}

// Duyệt trung thứ tự: in các khóa tăng dần
void tree_print(const Node *root) {
    if (!root) return;
    tree_print(root->left);
    printf("%d ", root->key);
    tree_print(root->right);
}

// Giải phóng: phải free con trước, rồi mới free cha (thứ tự hậu tố)
void tree_free(Node *root) {
    if (!root) return;
    tree_free(root->left);
    tree_free(root->right);
    free(root);
}

int main(void) {
    int keys[] = {50, 30, 70, 20, 40, 60, 80, 30};
    Node *root = NULL;
    for (size_t i = 0; i < sizeof keys / sizeof keys[0]; i++)
        root = tree_insert(root, keys[i]);

    tree_print(root);                                   // 20 30 40 50 60 70 80
    printf("\ncontains 60: %d, contains 65: %d\n", tree_contains(root, 60), tree_contains(root, 65));

    tree_free(root);
    return 0;
}
