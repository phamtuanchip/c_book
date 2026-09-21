// recursion_strings.c
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void reverse_rec(char *s, size_t lo, size_t hi) {    // đảo đoạn s[lo..hi]
    if (lo >= hi) return;
    char t = s[lo]; s[lo] = s[hi]; s[hi] = t;
    reverse_rec(s, lo + 1, hi - 1);
}

// Bỏ qua ký tự không phải chữ/số và phân biệt hoa/thường không quan trọng
static int is_palindrome(const char *s, size_t lo, size_t hi) {
    while (lo < hi && !isalnum((unsigned char)s[lo])) lo++;
    while (lo < hi && !isalnum((unsigned char)s[hi])) hi--;
    if (lo >= hi) return 1;
    if (tolower((unsigned char)s[lo]) != tolower((unsigned char)s[hi])) return 0;
    return is_palindrome(s, lo + 1, hi - 1);
}

int main(void) {
    char s[] = "abcdef";
    reverse_rec(s, 0, strlen(s) - 1);
    printf("%s\n", s);                                                        // fedcba

    const char *p = "A man, a plan, a canal: Panama";
    printf("%d\n", is_palindrome(p, 0, strlen(p) - 1));                       // 1
    return 0;
}
