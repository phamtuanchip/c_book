// text_stats.c
#include <stdio.h>
#include <ctype.h>
#include <string.h>

int main(void) {
    char line[1024];
    int freq[26] = {0};       // freq[0] ứng với 'a', ..., freq[25] ứng với 'z'
    int words = 0, chars = 0;

    while (fgets(line, sizeof line, stdin) != NULL) {
        int in_word = 0;
        for (size_t i = 0; line[i] != '\0'; i++) {
            unsigned char c = (unsigned char)line[i];
            chars++;
            if (isalpha(c)) {
                freq[tolower(c) - 'a']++;
            }
            if (isspace(c)) {
                in_word = 0;
            } else if (!in_word) {
                in_word = 1;
                words++;          // bắt đầu một từ mới
            }
        }
    }

    printf("So ky tu: %d, so tu: %d\n", chars, words);
    for (int i = 0; i < 26; i++) {
        if (freq[i] > 0) printf("%c: %d\n", 'a' + i, freq[i]);
    }
    return 0;
}
