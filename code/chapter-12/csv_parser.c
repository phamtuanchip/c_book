#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char line[1024];
    printf("Enter CSV lines (ctrl+D to end):\n");
    while (fgets(line, sizeof(line), stdin)) {
        // simple split by comma, not handling quoted fields
        char *p = strtok(line, ",\n");
        int col = 0;
        while (p) {
            printf("col %d: '%s'\n", col++, p);
            p = strtok(NULL, ",\n");
        }
    }
    return 0;
}
