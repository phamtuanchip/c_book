// args.c
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // (a) in ngược
    for (int i = argc - 1; i >= 1; i--) printf("%s ", argv[i]);
    printf("\n");

    // (b) cộng các số
    long sum = 0;
    for (int i = 1; i < argc; i++) {
        char *end;
        errno = 0;
        long v = strtol(argv[i], &end, 10);
        if (end == argv[i] || *end != '\0' || errno == ERANGE) {
            fprintf(stderr, "khong phai so: %s\n", argv[i]);
            return 1;
        }
        sum += v;
    }
    printf("tong = %ld\n", sum);
    return 0;
}
