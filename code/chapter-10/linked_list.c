#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    int value;
    struct Node *next;
} Node;

Node* push_front(Node *head, int v) {
    Node *n = malloc(sizeof(Node));
    if (!n) return head;
    n->value = v; n->next = head; return n;
}

Node* find(Node *head, int v) {
    while (head) { if (head->value == v) return head; head = head->next; }
    return NULL;
}

Node* remove_value(Node *head, int v) {
    Node **pp = &head;
    while (*pp) {
        if ((*pp)->value == v) {
            Node *tmp = *pp;
            *pp = tmp->next;
            free(tmp);
            break;
        }
        pp = &(*pp)->next;
    }
    return head;
}

void free_list(Node *head) {
    while (head) { Node *t = head; head = head->next; free(t); }
}

int main(void) {
    Node *list = NULL;
    list = push_front(list, 3);
    list = push_front(list, 5);
    list = push_front(list, 7);
    Node *f = find(list, 5);
    printf("Found: %d\n", f ? f->value : -1);
    list = remove_value(list, 5);
    free_list(list);
    return 0;
}
