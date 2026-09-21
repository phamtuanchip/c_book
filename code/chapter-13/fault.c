// fault.c
#include <stdlib.h>
#include "fault.h"
int g_fail_after = -1;                 // -1: không chèn lỗi

void *test_malloc(size_t n) {
    if (g_fail_after == 0) return NULL;          // giả lập hết bộ nhớ
    if (g_fail_after > 0) g_fail_after--;
    return malloc(n);
}
