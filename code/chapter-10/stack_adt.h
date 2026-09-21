// stack_adt.h
typedef struct Stack Stack;               // khai báo kiểu, KHÔNG lộ trường

Stack *stack_create(void);
void   stack_free(Stack *s);
int    stack_push(Stack *s, int v);
int    stack_pop(Stack *s, int *out);
