# Chương 19 — Project: Trình biên dịch / Interpreter mini

## Mục tiêu chương

- Xây dựng từng bước một **trình thông dịch** cho ngôn ngữ nhỏ: **lexer → parser → AST → evaluator**, rồi mở rộng sang **bytecode + máy ảo (VM)**.
- Hiểu **ngữ pháp** (EBNF), **độ ưu tiên toán tử** và cách **đệ quy xuống (recursive descent)** hiện thực chúng.
- Áp dụng những gì đã học: `struct`, `union`/tagged, con trỏ, cấp phát động, đệ quy, xử lý lỗi, kiểm thử.
- Quản lý bộ nhớ của cây cú pháp (tạo và giải phóng đúng) và báo lỗi có **vị trí** dễ hiểu.
- Biết hướng mở rộng: biến, `if`/`while`, hàm.

> **Cách làm chương này:** đọc phần thiết kế, rồi **gõ và chạy từng giai đoạn** (lexer trước, parser sau, ...). Mã hoàn chỉnh của cả bốn giai đoạn ở mục 19.7; các phần trước chia nhỏ để bạn hiểu và kiểm thử từng bước.

## 19.1. Trình thông dịch hoạt động ra sao?

Cả compiler lẫn interpreter đều biến **văn bản chương trình** thành **hành động**. Chúng chia công việc thành nhiều giai đoạn:

```text
"x = 1 + 2 * 3"
      │
      ▼  Lexer (phân tích từ vựng)
[IDENT x] [ASSIGN] [NUM 1] [PLUS] [NUM 2] [STAR] [NUM 3] [EOF]        ← dãy token
      │
      ▼  Parser (phân tích cú pháp)
            Assign(x)
                 │
               Add
              /    \
           Num 1    Mul
                   /   \
                Num 2  Num 3                                            ← cây cú pháp trừu tượng (AST)
      │
      ▼  Evaluator (duyệt cây và tính)          hoặc     Codegen → Bytecode → VM
   x = 7
```

| Giai đoạn | Đầu vào | Đầu ra | Việc làm |
|---|---|---|---|
| **Lexer** | chuỗi ký tự | dãy **token** | gom ký tự thành "từ": số, tên, toán tử; bỏ khoảng trắng, comment |
| **Parser** | dãy token | **AST** | kiểm tra cấu trúc đúng ngữ pháp và dựng cây thể hiện cấu trúc/độ ưu tiên |
| **Evaluator** | AST | giá trị | duyệt cây, tính toán ("tree-walking interpreter") |
| **Codegen + VM** | AST | bytecode → chạy | dịch cây thành lệnh đơn giản rồi chạy trên máy ảo ngăn xếp |

Một **interpreter** chạy chương trình trực tiếp (như trên); một **compiler** dịch nó thành mã đích (mã máy hoặc bytecode) để chạy sau. Phần 19.6 (bytecode + VM) nằm ở giữa: nó biên dịch sang bytecode rồi thông dịch bytecode đó — như Python, Lua, Java.

## 19.2. Thiết kế ngôn ngữ con

Ta gọi ngôn ngữ là **Calc**. Bước 1 hỗ trợ:

- Số thực: `3`, `2.5`
- Toán tử: `+ - * /`, dấu trừ một ngôi `-x`, ngoặc `( )`
- Biến và phép gán: `x = 1 + 2`
- Comment bắt đầu bằng `#`
- Mỗi dòng nhập là một lệnh (REPL)

Ví dụ phiên làm việc mong muốn:

```text
> x = 1 + 2 * (3 - 4)
= -1
> y = x * 10
= -10
> y / 4 + 1
= -1.5
> 1 / 0
loi: chia cho 0
> 2 +
loi: cot 4: can mot bieu thuc
```

### Ngữ pháp (EBNF)

Ngữ pháp mô tả **chính xác** những gì hợp lệ. Ký hiệu: `{ x }` = lặp 0 lần trở lên, `|` = hoặc, `'...'` = ký tự thật.

```text
statement  = IDENT '=' expr
           | expr ;

expr       = term   { ( '+' | '-' ) term   } ;
term       = unary  { ( '*' | '/' ) unary  } ;
unary      = '-' unary
           | primary ;
primary    = NUMBER
           | IDENT
           | '(' expr ')' ;
```

**Độ ưu tiên được mã hóa trong cấu trúc ngữ pháp:** `*` `/` nằm ở tầng `term` (sâu hơn) nên gắn chặt hơn `+` `-` ở tầng `expr`. Dấu ngoặc trong `primary` đưa lại một `expr` đầy đủ, nên ngoặc thay đổi độ ưu tiên. Vòng lặp `{ ... }` cho **kết hợp trái**: `1 - 2 - 3` = `(1 - 2) - 3` = −4.

Quy tắc chuyển ngữ pháp thành mã (recursive descent): **mỗi ký hiệu không kết thúc (như `expr`) là một hàm**; `{ ... }` thành vòng `while`; `|` thành `if` dựa vào token hiện tại.

## 19.3. Giai đoạn 1: Lexer

### Kiểu dữ liệu

```c
typedef enum {
    T_NUM, T_IDENT,
    T_PLUS, T_MINUS, T_STAR, T_SLASH,
    T_LPAREN, T_RPAREN, T_ASSIGN,
    T_EOF, T_ERROR
} TokType;

typedef struct {
    TokType type;
    double  num;          // giá trị khi type == T_NUM
    char    text[32];     // tên khi type == T_IDENT
    size_t  pos;          // vị trí (cột) bắt đầu trong dòng, để báo lỗi
} Token;

typedef struct {
    const char *src;      // toàn bộ dòng cần phân tích
    size_t      pos;      // vị trí hiện tại
} Lexer;
```

Lexer là một struct nhỏ có thể **sao chép theo giá trị** — tiện cho việc "nhìn trước" (lookahead) mà không tiêu thụ token.

### Hàm `lex_next`

```c
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static Token lex_next(Lexer *lx) {
    Token t;
    memset(&t, 0, sizeof t);

    // 1) bỏ khoảng trắng và comment ('#' đến hết dòng)
    for (;;) {
        while (isspace((unsigned char)lx->src[lx->pos])) lx->pos++;
        if (lx->src[lx->pos] == '#') {
            while (lx->src[lx->pos] != '\0' && lx->src[lx->pos] != '\n') lx->pos++;
        } else {
            break;
        }
    }

    t.pos = lx->pos;
    char c = lx->src[lx->pos];

    // 2) hết đầu vào
    if (c == '\0') { t.type = T_EOF; return t; }

    // 3) số: chữ số, hoặc '.' theo sau là chữ số
    if (isdigit((unsigned char)c) ||
        (c == '.' && isdigit((unsigned char)lx->src[lx->pos + 1]))) {
        char *end;
        t.num = strtod(lx->src + lx->pos, &end);        // strtod đọc cả phần thập phân/mũ
        lx->pos = (size_t)(end - lx->src);
        t.type = T_NUM;
        return t;
    }

    // 4) định danh: chữ cái hoặc '_' rồi chữ/số/'_'
    if (isalpha((unsigned char)c) || c == '_') {
        size_t n = 0;
        while (isalnum((unsigned char)lx->src[lx->pos]) || lx->src[lx->pos] == '_') {
            if (n + 1 >= sizeof t.text) { t.type = T_ERROR; return t; }   // tên quá dài
            t.text[n++] = lx->src[lx->pos++];
        }
        t.text[n] = '\0';
        t.type = T_IDENT;
        return t;
    }

    // 5) toán tử và dấu câu một ký tự
    lx->pos++;
    switch (c) {
        case '+': t.type = T_PLUS;   break;
        case '-': t.type = T_MINUS;  break;
        case '*': t.type = T_STAR;   break;
        case '/': t.type = T_SLASH;  break;
        case '(': t.type = T_LPAREN; break;
        case ')': t.type = T_RPAREN; break;
        case '=': t.type = T_ASSIGN; break;
        default:  t.type = T_ERROR;  break;    // ký tự lạ
    }
    return t;
}
```

Các quyết định thiết kế:

- **`strtod`** đọc số thực chuẩn (kể cả `1e3`) và cho ta biết nó dừng ở đâu (`end`).
- Ký tự lạ tạo token `T_ERROR` thay vì dừng chương trình — parser sẽ quyết định báo lỗi thế nào.
- Mỗi token nhớ `pos` để báo lỗi kiểu "cột 4".
- Ép `unsigned char` trước khi gọi `isdigit`, `isalpha`... (chương 2).

### Kiểm thử lexer riêng

Đừng viết parser trước khi chắc chắn lexer đúng. Một hàm in token:

```c
static const char *tok_name(TokType t) {
    static const char *names[] = {
        "NUM", "IDENT", "PLUS", "MINUS", "STAR", "SLASH", "LPAREN", "RPAREN", "ASSIGN", "EOF", "ERROR"
    };
    return names[t];
}

void dump_tokens(const char *line) {
    Lexer lx = { line, 0 };
    for (;;) {
        Token t = lex_next(&lx);
        printf("%s", tok_name(t.type));
        if (t.type == T_NUM)   printf("(%g)", t.num);
        if (t.type == T_IDENT) printf("(%s)", t.text);
        printf(" ");
        if (t.type == T_EOF || t.type == T_ERROR) break;
    }
    printf("\n");
}
```

Với `dump_tokens("x = 1.5 * (y + 2)")` phải in:

```text
IDENT(x) ASSIGN NUM(1.5) STAR LPAREN IDENT(y) PLUS NUM(2) RPAREN EOF
```

Các trường hợp biên cần thử: chuỗi rỗng, chỉ khoảng trắng, `.5`, `3.`, `1e3`, tên biến dài 40 ký tự, `@`, comment cuối dòng.

## 19.4. Giai đoạn 2: Parser và AST

### AST

Mỗi loại nút của cây là một trường hợp của `struct Node`. Để đơn giản, dùng một struct chung có các trường tùy loại:

```c
typedef enum { N_NUM, N_VAR, N_NEG, N_BINOP, N_ASSIGN } NodeKind;

typedef struct Node {
    NodeKind kind;
    double   num;               // N_NUM
    char     name[32];          // N_VAR, N_ASSIGN
    char     op;                // N_BINOP: '+', '-', '*', '/'
    struct Node *lhs, *rhs;     // N_BINOP: hai vế; N_NEG: lhs; N_ASSIGN: lhs = biểu thức giá trị
} Node;
```

(Với ngôn ngữ nhiều loại nút hơn, nên dùng *tagged union* — chương 10 — để tiết kiệm bộ nhớ và rõ ràng.)

Các hàm dựng nút và **giải phóng cây** (đệ quy, hậu tố):

```c
static Node *node_new(NodeKind kind) {
    Node *n = calloc(1, sizeof *n);          // calloc: mọi trường bằng 0/NULL
    if (n) n->kind = kind;
    return n;
}

static void node_free(Node *n) {
    if (!n) return;
    node_free(n->lhs);
    node_free(n->rhs);
    free(n);
}

// Dựng nút hai ngôi. Nếu hết bộ nhớ, giải phóng luôn hai vế để không rò rỉ.
static Node *node_binop(char op, Node *l, Node *r) {
    Node *n = node_new(N_BINOP);
    if (!n) { node_free(l); node_free(r); return NULL; }
    n->op = op; n->lhs = l; n->rhs = r;
    return n;
}
```

**Quy ước sở hữu:** khi `node_binop` thành công, nút mới **sở hữu** `l` và `r`; khi thất bại nó giải phóng chúng — nên người gọi không phải lo. Đây là ví dụ ownership rõ ràng (chương 9).

### Parser: trạng thái và báo lỗi

```c
typedef struct {
    Lexer  lx;
    Token  cur;                 // token hiện tại (đã đọc, chưa "ăn")
    int    failed;
    char   msg[96];
    size_t err_pos;
} Parser;

static void advance(Parser *p) { p->cur = lex_next(&p->lx); }

// Ghi lỗi ĐẦU TIÊN (các lỗi sau chỉ là hệ quả của lỗi đầu)
static void fail(Parser *p, const char *msg) {
    if (p->failed) return;
    p->failed = 1;
    p->err_pos = p->cur.pos;
    snprintf(p->msg, sizeof p->msg, "%s", msg);
}
```

### Các hàm phân tích — mỗi luật ngữ pháp một hàm

```c
static Node *parse_expr(Parser *p);         // khai báo trước vì primary gọi ngược lên expr

// primary = NUMBER | IDENT | '(' expr ')'
static Node *parse_primary(Parser *p) {
    if (p->cur.type == T_NUM) {
        Node *n = node_new(N_NUM);
        if (!n) { fail(p, "het bo nho"); return NULL; }
        n->num = p->cur.num;
        advance(p);
        return n;
    }
    if (p->cur.type == T_IDENT) {
        Node *n = node_new(N_VAR);
        if (!n) { fail(p, "het bo nho"); return NULL; }
        snprintf(n->name, sizeof n->name, "%s", p->cur.text);
        advance(p);
        return n;
    }
    if (p->cur.type == T_LPAREN) {
        advance(p);
        Node *inner = parse_expr(p);
        if (!inner) return NULL;
        if (p->cur.type != T_RPAREN) {
            fail(p, "thieu dau ')'");
            node_free(inner);
            return NULL;
        }
        advance(p);
        return inner;
    }
    fail(p, "can mot bieu thuc");
    return NULL;
}

// unary = '-' unary | primary
static Node *parse_unary(Parser *p) {
    if (p->cur.type == T_MINUS) {
        advance(p);
        Node *operand = parse_unary(p);            // đệ quy: cho phép "--x"
        if (!operand) return NULL;
        Node *n = node_new(N_NEG);
        if (!n) { fail(p, "het bo nho"); node_free(operand); return NULL; }
        n->lhs = operand;
        return n;
    }
    return parse_primary(p);
}

// term = unary { ('*' | '/') unary }
static Node *parse_term(Parser *p) {
    Node *lhs = parse_unary(p);
    while (lhs && (p->cur.type == T_STAR || p->cur.type == T_SLASH)) {
        char op = (p->cur.type == T_STAR) ? '*' : '/';
        advance(p);
        Node *rhs = parse_unary(p);
        if (!rhs) { node_free(lhs); return NULL; }
        lhs = node_binop(op, lhs, rhs);            // kết hợp trái: lhs mới chứa lhs cũ
        if (!lhs) fail(p, "het bo nho");
    }
    return lhs;
}

// expr = term { ('+' | '-') term }
static Node *parse_expr(Parser *p) {
    Node *lhs = parse_term(p);
    while (lhs && (p->cur.type == T_PLUS || p->cur.type == T_MINUS)) {
        char op = (p->cur.type == T_PLUS) ? '+' : '-';
        advance(p);
        Node *rhs = parse_term(p);
        if (!rhs) { node_free(lhs); return NULL; }
        lhs = node_binop(op, lhs, rhs);
        if (!lhs) fail(p, "het bo nho");
    }
    return lhs;
}

// statement = IDENT '=' expr | expr     (cần nhìn trước 1 token để phân biệt)
static Node *parse_statement(Parser *p) {
    if (p->cur.type == T_IDENT) {
        Lexer peek = p->lx;                          // sao chép lexer: nhìn trước mà không tiêu thụ
        if (lex_next(&peek).type == T_ASSIGN) {
            Node *n = node_new(N_ASSIGN);
            if (!n) { fail(p, "het bo nho"); return NULL; }
            snprintf(n->name, sizeof n->name, "%s", p->cur.text);
            advance(p);                              // ăn IDENT
            advance(p);                              // ăn '='
            n->lhs = parse_expr(p);
            if (!n->lhs) { node_free(n); return NULL; }
            return n;
        }
    }
    return parse_expr(p);
}

// Điểm vào: phân tích cả dòng; trả NULL và điền *err/*err_pos nếu lỗi
static Node *parse_line(const char *line, char *err, size_t err_cap, size_t *err_pos) {
    Parser p;
    memset(&p, 0, sizeof p);
    p.lx.src = line;
    advance(&p);                                     // nạp token đầu tiên

    Node *n = parse_statement(&p);
    if (n && p.cur.type != T_EOF) {                  // còn token thừa: "1 2", "1 )"
        fail(&p, "ky tu thua sau bieu thuc");
        node_free(n);
        n = NULL;
    }
    if (!n) {
        snprintf(err, err_cap, "%s", p.failed ? p.msg : "loi khong ro");
        *err_pos = p.err_pos;
    }
    return n;
}
```

### Vì sao recursive descent xử lý đúng độ ưu tiên?

Phân tích `1 + 2 * 3`:

1. `parse_expr` gọi `parse_term` → `parse_unary` → `parse_primary` đọc `1`.
2. Trong `parse_term`, token kế là `+` (không phải `*`/`/`) nên trả `1` về `parse_expr`.
3. `parse_expr` thấy `+`, đọc phần phải bằng `parse_term`: `parse_term` đọc `2`, thấy `*`, đọc `3`, dựng `Mul(2, 3)`.
4. `parse_expr` dựng `Add(1, Mul(2, 3))`.

Nhờ `*` được xử lý ở tầng sâu hơn, phép nhân "chiếm" toán hạng của nó trước phép cộng. **Kết hợp trái** đến từ vòng `while` (`lhs = binop(lhs, rhs)`): `8 - 3 - 2` dựng `Sub(Sub(8, 3), 2)` = 3, không phải `Sub(8, Sub(3, 2))` = 7.

### Hiển thị AST để kiểm thử

Kiểm thử parser bằng cách in cây dưới dạng chuỗi có ngoặc đầy đủ (S-expression):

```c
static void ast_print(const Node *n) {
    switch (n->kind) {
        case N_NUM:    printf("%g", n->num); break;
        case N_VAR:    printf("%s", n->name); break;
        case N_NEG:    printf("(neg "); ast_print(n->lhs); printf(")"); break;
        case N_BINOP:  printf("(%c ", n->op); ast_print(n->lhs); printf(" "); ast_print(n->rhs); printf(")"); break;
        case N_ASSIGN: printf("(= %s ", n->name); ast_print(n->lhs); printf(")"); break;
    }
}
```

Bảng kiểm thử (đầu vào → AST mong đợi):

| Đầu vào | AST |
|---|---|
| `1 + 2 * 3` | `(+ 1 (* 2 3))` |
| `(1 + 2) * 3` | `(* (+ 1 2) 3)` |
| `8 - 3 - 2` | `(- (- 8 3) 2)` |
| `-2 * 3` | `(* (neg 2) 3)` |
| `x = 1 + 2` | `(= x (+ 1 2))` |
| `2 +` | lỗi: cần một biểu thức |
| `(1 + 2` | lỗi: thiếu `)` |
| `1 2` | lỗi: ký tự thừa |

## 19.5. Giai đoạn 3: Evaluator (duyệt cây)

Cần một **môi trường (environment)** ánh xạ tên biến → giá trị:

```c
#define MAX_VARS 64

typedef struct { char name[32]; double value; } Var;
typedef struct { Var vars[MAX_VARS]; size_t count; } Env;

static Var *env_find(Env *e, const char *name) {
    for (size_t i = 0; i < e->count; i++)
        if (strcmp(e->vars[i].name, name) == 0) return &e->vars[i];
    return NULL;
}

// Trả 0 nếu thành công, -1 nếu môi trường đầy
static int env_set(Env *e, const char *name, double value) {
    Var *v = env_find(e, name);
    if (!v) {
        if (e->count == MAX_VARS) return -1;
        v = &e->vars[e->count++];
        snprintf(v->name, sizeof v->name, "%s", name);
    }
    v->value = value;
    return 0;
}
```

(Tìm tuyến tính O(n) là đủ cho 64 biến; nếu cần nhiều hơn, dùng bảng băm — bài tập chương 10.)

Hàm `eval` **đệ quy theo hình dạng của cây**: giá trị của một nút được tính từ giá trị các con.

```c
// Trả 0 và ghi kết quả vào *out nếu thành công; trả -1 và *err trỏ tới chuỗi hằng mô tả lỗi.
static int eval(const Node *n, Env *env, double *out, const char **err) {
    switch (n->kind) {
        case N_NUM:
            *out = n->num;
            return 0;

        case N_VAR: {
            Var *v = env_find(env, n->name);
            if (!v) { *err = "bien chua duoc dinh nghia"; return -1; }
            *out = v->value;
            return 0;
        }

        case N_NEG: {
            double x;
            if (eval(n->lhs, env, &x, err) != 0) return -1;
            *out = -x;
            return 0;
        }

        case N_BINOP: {
            double a, b;
            if (eval(n->lhs, env, &a, err) != 0) return -1;      // đánh giá vế trái trước
            if (eval(n->rhs, env, &b, err) != 0) return -1;
            switch (n->op) {
                case '+': *out = a + b; return 0;
                case '-': *out = a - b; return 0;
                case '*': *out = a * b; return 0;
                case '/':
                    if (b == 0.0) { *err = "chia cho 0"; return -1; }
                    *out = a / b;
                    return 0;
            }
            *err = "toan tu khong hop le";
            return -1;
        }

        case N_ASSIGN: {
            double v;
            if (eval(n->lhs, env, &v, err) != 0) return -1;
            if (env_set(env, n->name, v) != 0) { *err = "qua nhieu bien"; return -1; }
            *out = v;
            return 0;
        }
    }
    *err = "nut khong hop le";
    return -1;
}
```

Lưu ý **mẫu xử lý lỗi**: mọi hàm trả `0`/`-1`, kết quả qua tham số đầu ra (chương 13); lỗi lan truyền lên trên từng tầng cho tới REPL — không dùng `exit()` sâu bên trong.

## 19.6. Giai đoạn 4: Bytecode và máy ảo ngăn xếp

Trình duyệt cây chạy chậm vì mỗi phép tính đi qua nhiều lời gọi hàm đệ quy và con trỏ. Cách phổ biến để nhanh hơn: **biên dịch AST thành bytecode tuyến tính** rồi chạy bằng một vòng lặp đơn giản trên **máy ảo ngăn xếp (stack VM)**.

Ý tưởng: biểu thức hậu tố. `1 + 2 * 3` → `PUSH 1, PUSH 2, PUSH 3, MUL, ADD`:

```text
lệnh       ngăn xếp sau khi chạy
PUSH 1     [1]
PUSH 2     [1, 2]
PUSH 3     [1, 2, 3]
MUL        [1, 6]        (lấy 3 và 2, đẩy 2*3)
ADD        [7]           (lấy 6 và 1, đẩy 1+6)
```

Duyệt AST theo **thứ tự hậu tố** (con trước, nút sau) sinh đúng chuỗi này.

```c
typedef enum { OP_PUSH, OP_LOAD, OP_STORE, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_NEG, OP_HALT } OpCode;

typedef struct {
    OpCode op;
    double num;         // OP_PUSH
    int    slot;        // OP_LOAD / OP_STORE: chỉ số biến (đã đổi từ tên -> số lúc biên dịch)
} Instr;

typedef struct {
    Instr  code[256];
    size_t len;
    char   names[MAX_VARS][32];     // slot -> tên biến
    size_t nnames;
} Program;

static int emit(Program *p, Instr in) {
    if (p->len == sizeof p->code / sizeof p->code[0]) return -1;   // chương trình quá dài
    p->code[p->len++] = in;
    return 0;
}

// Tìm slot của biến đã có; -1 nếu chưa có
static int slot_find(const Program *p, const char *name) {
    for (size_t i = 0; i < p->nnames; i++)
        if (strcmp(p->names[i], name) == 0) return (int)i;
    return -1;
}

// Tìm hoặc tạo slot (dùng khi GÁN)
static int slot_of(Program *p, const char *name) {
    int s = slot_find(p, name);
    if (s >= 0) return s;
    if (p->nnames == MAX_VARS) return -1;
    snprintf(p->names[p->nnames], sizeof p->names[0], "%s", name);
    return (int)p->nnames++;
}

// Sinh mã cho nút n. Trả 0 nếu thành công.
static int compile(const Node *n, Program *p) {
    switch (n->kind) {
        case N_NUM:  return emit(p, (Instr){ .op = OP_PUSH, .num = n->num });
        case N_VAR: {
            int s = slot_find(p, n->name);               // ĐỌC biến chưa có là lỗi (giống evaluator)
            return s < 0 ? -1 : emit(p, (Instr){ .op = OP_LOAD, .slot = s });
        }
        case N_NEG:
            if (compile(n->lhs, p) != 0) return -1;
            return emit(p, (Instr){ .op = OP_NEG });
        case N_BINOP: {
            if (compile(n->lhs, p) != 0 || compile(n->rhs, p) != 0) return -1;
            OpCode op = n->op == '+' ? OP_ADD : n->op == '-' ? OP_SUB : n->op == '*' ? OP_MUL : OP_DIV;
            return emit(p, (Instr){ .op = op });
        }
        case N_ASSIGN: {
            if (compile(n->lhs, p) != 0) return -1;
            int s = slot_of(p, n->name);
            if (s < 0) return -1;
            // STORE lấy đỉnh ngăn xếp đặt vào biến nhưng KHÔNG pop, để kết quả phép gán còn lại làm giá trị biểu thức
            return emit(p, (Instr){ .op = OP_STORE, .slot = s });
        }
    }
    return -1;
}

// Máy ảo: vòng lặp đọc-giải mã-thực thi
static int run(const Program *p, double vars[], double *result, const char **err) {
    double stack[64];
    size_t sp = 0;                                   // sp = số phần tử đang có

    for (size_t pc = 0; pc < p->len; pc++) {
        const Instr *in = &p->code[pc];
        switch (in->op) {
            case OP_PUSH:  if (sp == 64) { *err = "tran ngan xep"; return -1; }
                           stack[sp++] = in->num; break;
            case OP_LOAD:  if (sp == 64) { *err = "tran ngan xep"; return -1; }
                           stack[sp++] = vars[in->slot]; break;
            case OP_STORE: vars[in->slot] = stack[sp - 1]; break;
            case OP_NEG:   stack[sp - 1] = -stack[sp - 1]; break;
            case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: {
                double b = stack[--sp], a = stack[sp - 1];
                switch (in->op) {
                    case OP_ADD: a += b; break;
                    case OP_SUB: a -= b; break;
                    case OP_MUL: a *= b; break;
                    default:
                        if (b == 0.0) { *err = "chia cho 0"; return -1; }
                        a /= b;
                }
                stack[sp - 1] = a;
                break;
            }
            case OP_HALT: pc = p->len; break;
        }
    }
    if (sp != 1) { *err = "loi noi bo: ngan xep khong khop"; return -1; }
    *result = stack[0];
    return 0;
}

// Chạy một câu lệnh bằng VM nhưng dùng chung biến với Env, để so sánh với eval()
static int vm_eval(const Node *ast, Env *env, double *out, const char **err) {
    Program *p = calloc(1, sizeof *p);                 // Program khá lớn: cấp phát trên heap
    if (!p) { *err = "het bo nho"; return -1; }
    double vars[MAX_VARS] = {0};
    for (size_t i = 0; i < env->count; i++) {          // nạp biến hiện có vào các slot đầu tiên
        snprintf(p->names[i], sizeof p->names[i], "%s", env->vars[i].name);
        vars[i] = env->vars[i].value;
    }
    p->nnames = env->count;

    int rc = -1;
    if (compile(ast, p) != 0)      *err = "bien chua duoc dinh nghia (hoac chuong trinh qua dai)";
    else if (run(p, vars, out, err) == 0) {
        rc = 0;
        for (size_t i = 0; i < p->nnames; i++)         // ghi ngược giá trị biến vào Env
            if (env_set(env, p->names[i], vars[i]) != 0) { *err = "qua nhieu bien"; rc = -1; break; }
    }
    free(p);
    return rc;
}
```

Điểm đáng chú ý:

- Tên biến được đổi thành **chỉ số (slot)** lúc biên dịch → truy cập biến là `vars[slot]` (O(1)) thay vì so chuỗi.
- Vòng lặp `run` là "trái tim" của mọi VM: **fetch–decode–execute**. Các VM thực tế dùng kỹ thuật như *computed goto* để nhanh hơn.
- Hằng số lưu trực tiếp trong lệnh; VM thực tế có bảng hằng và lệnh có độ dài thay đổi.
- Một bất biến cần kiểm tra: sau khi chạy xong một biểu thức, ngăn xếp có **đúng 1** phần tử.
- `vm_eval` bọc `compile` + `run` để dùng chung `Env` với evaluator: chạy `./calc --vm` để REPL dùng VM, và **so sánh kết quả hai cách** (kiểm thử vi sai, mục 19.8).

## 19.7. Chương trình hoàn chỉnh (REPL)

Ghép tất cả thành `calc.c`. Bạn đã có các phần: kiểu token, `lex_next`, `Node`, parser, `Env`, `eval`. Thêm hàm `main` dưới đây và xếp mã theo thứ tự: `#include` → kiểu token & lexer → AST & parser → môi trường & `eval` → `main`.

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ... dán vào: TokType, Token, Lexer, lex_next ... */
/* ... dán vào: Node, node_new, node_free, node_binop, Parser, advance, fail, parse_* , parse_line ... */
/* ... dán vào: Var, Env, env_find, env_set, eval ... */
/* ... dán vào (tùy chọn): OpCode, Instr, Program, emit, slot_find, slot_of, compile, run, vm_eval ... */

int main(int argc, char *argv[]) {
    int use_vm = argc > 1 && strcmp(argv[1], "--vm") == 0;      // ./calc --vm: chạy bằng bytecode VM
    Env env;
    memset(&env, 0, sizeof env);

    char line[256];
    for (;;) {
        printf("> ");
        fflush(stdout);
        if (!fgets(line, sizeof line, stdin)) break;                 // EOF (Ctrl+D / Ctrl+Z)

        line[strcspn(line, "\r\n")] = '\0';
        const char *s = line;
        while (isspace((unsigned char)*s)) s++;
        if (*s == '\0' || *s == '#') continue;                       // dòng trống hoặc chỉ có comment
        if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) break;

        char err[96];
        size_t err_pos = 0;
        Node *ast = parse_line(line, err, sizeof err, &err_pos);
        if (!ast) {
            printf("loi: cot %zu: %s\n", err_pos + 1, err);
            continue;
        }

        double value;
        const char *rt_err = NULL;
        int ok = use_vm ? vm_eval(ast, &env, &value, &rt_err) : eval(ast, &env, &value, &rt_err);
        if (ok == 0) printf("= %g\n", value);
        else         printf("loi: %s\n", rt_err);

        node_free(ast);                                              // luôn giải phóng cây sau mỗi dòng
    }
    return 0;
}
```

Biên dịch và kiểm tra bộ nhớ:

```bash
gcc -std=c11 -Wall -Wextra -g -fsanitize=address,undefined -o calc calc.c
./calc
> x = 1 + 2 * (3 - 4)
= -1
> y = x * 10
= -10
> y / 4 + 1
= -1.5
> 1 / 0
loi: chia cho 0
> 2 +
loi: cot 4: can mot bieu thuc
```

Chạy dưới ASan/LeakSanitizer bảo đảm cây được giải phóng đầy đủ trên mọi đường (thành công, lỗi cú pháp, lỗi chạy).

## 19.8. Kiểm thử từng giai đoạn

Phát triển ngôn ngữ là lĩnh vực **rất hợp với kiểm thử tự động**: đầu vào và đầu ra rõ ràng. Với `tiny_test.h` (chương 14) hoặc khung Unity (chương 21):

```c
// test_calc.c (ý tưởng)
static double run_expr(const char *src, Env *env, int *ok) {
    char err[96]; size_t pos;
    Node *ast = parse_line(src, err, sizeof err, &pos);
    if (!ast) { *ok = 0; return 0; }
    double v = 0; const char *e;
    *ok = (eval(ast, env, &v, &e) == 0);
    node_free(ast);
    return v;
}

int main(void) {
    Env env = {0};
    int ok;
    CHECK_EQ_INT((long long)run_expr("1 + 2 * 3", &env, &ok), 7);
    CHECK(ok);
    CHECK_EQ_INT((long long)run_expr("(1 + 2) * 3", &env, &ok), 9);
    CHECK_EQ_INT((long long)run_expr("8 - 3 - 2", &env, &ok), 3);       // kết hợp trái
    CHECK_EQ_INT((long long)run_expr("-2 * -3", &env, &ok), 6);
    run_expr("1 / 0", &env, &ok);        CHECK(!ok);                     // lỗi chạy
    run_expr("2 +", &env, &ok);          CHECK(!ok);                     // lỗi cú pháp
    run_expr("x = 5", &env, &ok);        CHECK(ok);
    CHECK_EQ_INT((long long)run_expr("x * x", &env, &ok), 25);
    run_expr("undefined_var", &env, &ok); CHECK(!ok);
    return TEST_SUMMARY();
}
```

Kỹ thuật kiểm thử tốt cho parser/interpreter:

- **Bảng (input, expected)**, thêm mỗi lần tìm ra lỗi mới (kiểm thử hồi quy).
- **Kiểm thử lỗi:** đầu vào sai có báo lỗi *đúng loại, đúng vị trí* và không crash/rò rỉ.
- **So sánh hai cách chạy:** kết quả của **evaluator** và **VM** phải giống nhau trên mọi biểu thức — cách bắt lỗi rất mạnh (kiểm thử vi sai, *differential testing*).
- **Fuzz** `parse_line` (chương 18) để bảo đảm không crash với đầu vào bất kỳ.

## 19.9. Mở rộng ngôn ngữ

Mỗi bước thêm một tính năng theo cùng khuôn: **ngữ pháp → lexer → parser → AST → eval → kiểm thử**.

### Bước A: hàm dựng sẵn (`sqrt`, `sin`) và lũy thừa

- Lexer: thêm token `T_COMMA`, `T_CARET`.
- Ngữ pháp: `primary = IDENT '(' [ expr { ',' expr } ] ')'`, và `power = unary [ '^' power ]` (kết hợp **phải**: `2^3^2 = 2^9`).
- Nút `N_CALL` chứa tên hàm và mảng đối số; `eval` tra bảng `{ "sqrt", sqrt }`.

### Bước B: so sánh, `if` và `while`

```text
statement = 'if' expr block [ 'else' block ]
          | 'while' expr block
          | IDENT '=' expr
          | expr ;
block     = '{' { statement ';' } '}' ;
compare   = expr [ ( '<' | '>' | '==' | '!=' ) expr ] ;
```

- Từ khóa: sau khi đọc IDENT, kiểm tra có phải `if`/`while`/`else` không.
- AST: `N_IF(cond, then, else)`, `N_WHILE(cond, body)`, `N_BLOCK(list)`.
- Với VM: cần lệnh **nhảy** `OP_JMP`, `OP_JMPZ` (nhảy nếu bằng 0). Dịch `while` thành:

```text
L1:  <điều kiện>
     JMPZ L2
     <thân>
     JMP L1
L2:
```

### Bước C: hàm do người dùng định nghĩa

```text
def add(a, b) { return a + b; }
```

Cần: **phạm vi biến** (mỗi lần gọi hàm có môi trường riêng — ngăn xếp các `Env`), **khung gọi (call frame)**, lệnh `return`, và (trên VM) lệnh `CALL`/`RET` với địa chỉ trở về. Đệ quy hoạt động "miễn phí" nếu mỗi lần gọi có frame riêng.

### Bước D: kiểu dữ liệu

Thay `double` bằng **tagged union** `Value { type; union { double d; char *s; bool b; } }` để có chuỗi/boolean. Chuỗi kéo theo **quản lý bộ nhớ** — bài toán thú vị: đếm tham chiếu hay bộ gom rác (mark-and-sweep)?

### Hướng xa hơn

- **Báo lỗi tốt hơn:** in dòng nguồn với dấu `^` chỉ đúng cột.
- **Tối ưu:** *constant folding* (`2 * 3` thành `6` ngay lúc biên dịch), loại bỏ mã chết.
- **Sinh mã máy thật:** xuất assembly x86-64/ARM hoặc LLVM IR; xem `Crafting Interpreters` (Robert Nystrom) và "Writing an Interpreter in Go" để tìm hiểu tiếp.
- **Công cụ sinh parser:** `flex`/`bison`, `ANTLR`, `re2c` — tốt cho ngôn ngữ lớn; viết tay recursive descent (như trên) vẫn được dùng trong nhiều compiler thực tế (GCC, Clang, V8...) vì kiểm soát lỗi tốt.

## 19.10. Lỗi thường gặp

| Lỗi | Triệu chứng | Cách tránh |
|---|---|---|
| Quên `advance()` sau khi dùng token | Parser lặp vô hạn hoặc phân tích sai | Mỗi lần khớp token phải "ăn" nó |
| Ngữ pháp đệ quy trái (`expr = expr '+' term`) | Đệ quy vô hạn, tràn stack | Viết lại thành vòng lặp `{ ... }` |
| Dùng đệ quy cho phép toán kết hợp trái | `8 - 3 - 2` ra 7 thay vì 3 | Dùng `while` và dựng cây tích lũy |
| Rò rỉ AST khi lỗi giữa chừng | LeakSanitizer báo `definitely lost` | Ownership rõ; giải phóng trên mọi đường lỗi |
| Không kiểm tra hết bộ nhớ khi dựng nút | Crash dưới `malloc` lỗi | `node_new` trả NULL, lan truyền |
| Chia cho 0 (`double`) cho `inf` thay vì lỗi | Kết quả lạ | Kiểm tra mẫu số |
| Báo nhiều lỗi dây chuyền cho một lỗi gốc | Thông báo khó hiểu | Chỉ giữ lỗi đầu tiên |
| Tràn ngăn xếp VM với biểu thức lồng sâu | Ghi ngoài mảng | Kiểm tra `sp` hoặc tính độ sâu tối đa lúc biên dịch |
| Không giới hạn độ sâu đệ quy của parser | Tràn stack với `((((...` | Đếm và từ chối độ sâu quá lớn |

## 19.11. Tóm tắt

- Trình thông dịch gồm **lexer → parser → AST → evaluator** (hoặc **codegen → VM**).
- **Ngữ pháp EBNF** mô tả ngôn ngữ; **recursive descent** biến mỗi luật thành một hàm, vòng `{ }` thành `while`, và độ ưu tiên đến từ việc phân tầng luật.
- AST là cây các `struct` cấp phát động: cần hàm giải phóng đệ quy và ownership rõ ràng.
- Evaluator đệ quy theo cấu trúc cây; lỗi lan truyền bằng mã trả về, không `exit()` sâu.
- Bytecode + VM ngăn xếp nhanh hơn duyệt cây; đổi tên biến thành chỉ số lúc biên dịch.
- Kiểm thử từng giai đoạn, kiểm thử lỗi, kiểm thử vi sai và fuzz giúp ngôn ngữ đáng tin cậy.

## 19.12. Bài tập dự án

**Bước 1 — Lexer.** Hoàn thiện `lex_next` và `dump_tokens`; viết bảng kiểm thử ít nhất 15 trường hợp (kể cả các trường hợp biên đã nêu). Thêm token cho số nguyên hex `0x1F` và chuỗi trong nháy kép.

**Bước 2 — Parser + AST + evaluator.** Ghép `calc.c` từ các mảnh trong chương; dùng ASan/LeakSanitizer để bảo đảm không rò rỉ; thêm `ast_print` và chế độ `:ast` in cây thay vì tính.

**Bước 3 — Cải thiện báo lỗi.** In dòng nhập kèm dấu `^` trỏ vào cột lỗi; phân biệt lỗi từ vựng, cú pháp, chạy.

**Bước 4 — Mở rộng biểu thức.** Thêm toán tử `%`, `^` (kết hợp phải), so sánh, hằng `pi`, và các hàm `sqrt`, `abs`, `min`, `max`.

**Bước 5 — Điều khiển luồng.** Thêm `if/else`, `while`, khối `{ }` và đọc chương trình nhiều dòng từ file (`./calc script.calc`).

**Bước 6 — Hàm.** Thêm `def`, tham số, `return`, phạm vi cục bộ; kiểm thử với `fib(n)` đệ quy và `gcd`.

**Bước 7 — Bytecode VM.** Hoàn thiện đường biên dịch–chạy ở 19.6 với cả biến, `if`, `while`; so sánh kết quả và **tốc độ** với evaluator trên vòng lặp 10 triệu lần; dùng `perf`/`gprof` (chương 17) để xem đâu là hot spot.

**Bước 8 — (Thử thách)** Thêm **constant folding** và chế độ `--dump-bytecode` in ra chương trình dưới dạng assembly dễ đọc; hoặc sinh mã C từ AST rồi gọi `gcc` biên dịch (một *transpiler*).

Mã nguồn mẫu: /code/chapter-19
