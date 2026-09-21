# Chương 19 — Project: Trình biên dịch / Interpreter mini

Mục tiêu chương:

- Xây một mini language step-by-step: lexer → parser → AST → evaluator/codegen.
- Viết tests cho từng bước để đảm bảo đúng và có thể mở rộng.

1. Thiết kế ngôn ngữ con

- Bắt đầu với biểu thức số học, biến, và assignment: e.g., x = 1 + 2 * (3 - 4)
- Quy định grammar đơn giản (EBNF) và precedence.

2. Lexical analysis

- Viết lexer trả về token types: NUMBER, IDENT, PLUS, MINUS, STAR, SLASH, LPAREN, RPAREN, ASSIGN, SEMI.
- Xử lý whitespace và comments.

3. Parsing

- Dùng recursive-descent parser: expr -> term { (+|-) term }
- Xây AST nodes: Number, BinaryOp, Variable, Assign

4. Evaluation / Codegen

- Interpreter: traverse AST và evaluate; giữ environment (map var->value).
- Optionally: codegen to stack bytecode and implement VM loop.

5. Testing & incremental development

- TDD: viết tests cho lexer, parser and evaluator.
- Keep interfaces small and well-documented.

6. Ví dụ thực hành

- /code/chapter-19/lexer.c
- /code/chapter-19/parser.c
- /code/chapter-19/eval.c (main runs REPL)

Bài tập dự án

- Bước 1: implement lexer and unit tests.
- Bước 2: parser + AST + evaluator; support variables.
- Bước 3: extend language: functions, if/else, while.

Ghi chú: kèm Makefile và test scripts in /code/chapter-19.