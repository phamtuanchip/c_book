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
