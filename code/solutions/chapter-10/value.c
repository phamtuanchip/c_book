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
