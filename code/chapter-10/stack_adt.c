// stack_adt.c
#include "stack_adt.h"
#include <stdlib.h>

struct Stack {                            // định nghĩa đầy đủ chỉ nằm trong .c
    int *data;
    size_t size, cap;
};
/* ... cài đặt ... */
