/* Deliberate buffer overflow example (educational) - do NOT use in production
Build: gcc -Wall -Wextra -std=c11 -o overflow buffer_overflow.c
Use ASAN to detect: gcc -fsanitize=address -g -o overflow buffer_overflow.c
*/
#include <stdio.h>
#include <string.h>

int main(void) {
    char buf[8];
    printf("Enter a string: ");
    gets(buf); // intentionally unsafe for demonstration only (gets is removed in modern standards)
    printf("You entered: %s\n", buf);
    return 0;
}
