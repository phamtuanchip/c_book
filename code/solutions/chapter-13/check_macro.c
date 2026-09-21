// check_macro.c
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr, rc_var, code)                                                   \
    do {                                                                            \
        if (!(expr)) {                                                              \
            fprintf(stderr, "%s:%d: '%s' that bai: %s\n", __FILE__, __LINE__, #expr, strerror(errno)); \
            (rc_var) = (code);                                                      \
            goto cleanup;                                                           \
        }                                                                           \
    } while (0)

int main(void) {
    int rc = 0;
    FILE *f = NULL;
    CHECK((f = fopen("khong_co.txt", "r")) != NULL, rc, 1);
    fclose(f);
cleanup:
    return rc;
}
