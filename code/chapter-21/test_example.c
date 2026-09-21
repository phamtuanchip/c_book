/* Simple unit test style example using assert
Build: gcc -Wall -Wextra -std=c11 -o test_example test_example.c
*/
#include <assert.h>
#include <stdio.h>

int add(int a, int b) { return a + b; }

void test_add(void) {
    assert(add(1,2) == 3);
    assert(add(-1,1) == 0);
}

int main(void) {
    test_add();
    printf("All tests passed\n");
    return 0;
}
