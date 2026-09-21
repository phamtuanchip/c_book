// brackets.c
#include <stdio.h>
#include <string.h>

static int balanced(const char *s) {
    char stack[256];
    size_t top = 0;
    for (; *s; s++) {
        char c = *s;
        if (c == '(' || c == '[' || c == '{') {
            if (top == sizeof stack) return 0;                 // quá sâu
            stack[top++] = c;
        } else if (c == ')' || c == ']' || c == '}') {
            if (top == 0) return 0;                            // đóng khi chưa mở
            char o = stack[--top];
            if ((c == ')' && o != '(') || (c == ']' && o != '[') || (c == '}' && o != '{')) return 0;
        }
    }
    return top == 0;                                           // không còn ngoặc mở dở
}

int main(void) {
    const char *tests[] = {"([]{})", "([)]", "((", "", "a(b)c"};
    for (size_t i = 0; i < sizeof tests / sizeof tests[0]; i++)
        printf("%-8s -> %s\n", tests[i], balanced(tests[i]) ? "can bang" : "sai");
    return 0;
}
