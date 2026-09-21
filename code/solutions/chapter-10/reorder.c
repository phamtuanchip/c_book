// reorder.c
#include <stddef.h>
#include <stdio.h>

struct Bad  { char a; double b; char c; int d; short e; };    // đệm nhiều
struct Good { double b; int d; short e; char a; char c; };    // giảm dần theo kích thước

int main(void) {
    printf("Bad  = %zu byte\n", sizeof(struct Bad));           // 32
    printf("Good = %zu byte\n", sizeof(struct Good));          // 16
    return 0;
}
