// trycatch.c
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_DEPTH 16
static jmp_buf g_stack[MAX_DEPTH];
static int g_depth = 0;

#define TRY   if (g_depth < MAX_DEPTH && setjmp(g_stack[g_depth++]) == 0) {
#define CATCH } else {
#define ENDTRY }
#define THROW(code) do { if (g_depth > 0) longjmp(g_stack[--g_depth], (code)); abort(); } while (0)

static void risky(int x) {
    if (x > 2) THROW(x);
}

int main(void) {
    TRY
        risky(1);
        risky(5);
        printf("khong toi day\n");
        g_depth--;                       // thoát khỏi TRY bình thường phải bỏ khung
    CATCH
        printf("bat duoc loi\n");
    ENDTRY
    return 0;
}
