Chapter 19 code samples - mini compiler/interpreter

Files:
- lexer.c: tiny lexer for arithmetic expressions (NUM, +, -, *, /, parentheses)

Build:
- gcc -Wall -Wextra -std=c11 -o lexer lexer.c

Run:
- echo "12 + 34 * (5 - 2)" | ./lexer

Notes: next steps: implement parser (recursive descent) and evaluator, build AST structures.
