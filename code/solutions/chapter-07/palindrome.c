// palindrome.c
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void reverse_string(char *s) {
    size_t n = strlen(s);
    for (size_t i = 0; i < n / 2; i++) {
        char t = s[i]; s[i] = s[n - 1 - i]; s[n - 1 - i] = t;
    }
}

static int is_palindrome(const char *s) {
    size_t i = 0, j = strlen(s);
    while (i < j) {
        if (!isalnum((unsigned char)s[i])) { i++; continue; }
        if (!isalnum((unsigned char)s[j - 1])) { j--; continue; }
        if (tolower((unsigned char)s[i]) != tolower((unsigned char)s[j - 1])) return 0;
        i++; j--;
    }
    return 1;
}

int main(void) {
    char s[] = "hello";
    reverse_string(s);
    printf("%s\n", s);                                            // olleh
    printf("%d %d\n", is_palindrome("Was it a car or a cat I saw?"), is_palindrome("abc"));   // 1 0
    return 0;
}
