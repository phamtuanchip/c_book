// demo_malloc.c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    size_t n = 5;
    int *arr = malloc(n * sizeof(int));
    if (arr == NULL) { perror("malloc"); return 1; }

    for (size_t i = 0; i < n; i++) arr[i] = (int)(i * i);

    // mở rộng lên 10 phần tử
    int *tmp = realloc(arr, 10 * sizeof(int));
    if (tmp == NULL) { free(arr); perror("realloc"); return 1; }
    arr = tmp;                                  // chỉ gán lại khi realloc thành công
    for (size_t i = n; i < 10; i++) arr[i] = (int)(i * i);

    for (size_t i = 0; i < 10; i++) printf("%d ", arr[i]);
    printf("\n");

    free(arr);
    arr = NULL;             // tránh con trỏ "treo" (dangling pointer)
    return 0;
}
