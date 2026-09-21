/* Tiny lexer for arithmetic expressions: tokens: NUM, +, -, *, /, LPAREN, RPAREN
Build: gcc -Wall -Wextra -std=c11 -o lexer lexer.c
*/
#include <stdio.h>
#include <ctype.h>

int main(void) {
    int c;
    while ((c = getchar()) != EOF) {
        if (isspace(c)) continue;
        if (isdigit(c)) {
            int v = c - '0';
            while (isdigit(c = getchar())) v = v*10 + (c - '0');
            ungetc(c, stdin);
            printf("NUM(%d) ", v);
            continue;
        }
        switch (c) {
            case '+': printf("PLUS "); break;
            case '-': printf("MINUS "); break;
            case '*': printf("MUL "); break;
            case '/': printf("DIV "); break;
            case '(': printf("LPAREN "); break;
            case ')': printf("RPAREN "); break;
            default: printf("UNKNOWN(%c) ", c); break;
        }
    }
    printf("\n");
    return 0;
}
