// ptr_practice.c
#include <stdio.h>
#include <string.h>

// Đảo mảng bằng hai con trỏ từ hai đầu
void reverse_ints(int *first, int *last) {     // last trỏ tới phần tử cuối
    while (first < last) {
        int t = *first;
        *first++ = *last;
        *last-- = t;
    }
}

// Tìm chuỗi con bằng con trỏ; trả về con trỏ tới lần xuất hiện đầu hoặc NULL
const char *find_sub(const char *hay, const char *needle) {
    if (*needle == '\0') return hay;
    for (; *hay; hay++) {
        const char *h = hay, *n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (*n == '\0') return hay;            // khớp hết needle
    }
    return NULL;
}

int main(void) {
    int a[] = {1, 2, 3, 4, 5};
    size_t n = sizeof a / sizeof a[0];
    reverse_ints(a, a + n - 1);
    for (size_t i = 0; i < n; i++) printf("%d ", a[i]);     // 5 4 3 2 1
    printf("\n");

    const char *text = "the quick brown fox";
    const char *pos = find_sub(text, "brown");
    if (pos) printf("tim thay tai chi so %td: %s\n", pos - text, pos);   // %td cho ptrdiff_t
    return 0;
}
