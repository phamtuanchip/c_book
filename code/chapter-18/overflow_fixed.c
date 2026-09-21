#include <stdio.h>
#include <string.h>

int main(void) {
    char buf[256];
    printf("Enter a string: ");
    if (fgets(buf, sizeof(buf), stdin) == NULL) return 1;
    buf[strcspn(buf, "\n")] = '\0';
    printf("You entered: %s\n", buf);
    return 0;
}
