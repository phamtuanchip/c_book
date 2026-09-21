// calc.c — trình thông dịch mini (ghép từ các mục 19.3–19.7)

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <ctype.h>



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

typedef enum { N_NUM, N_VAR, N_NEG, N_BINOP, N_ASSIGN } NodeKind;

typedef struct Node {
    NodeKind kind;
    double   num;               // N_NUM
    char     name[32];          // N_VAR, N_ASSIGN
    char     op;                // N_BINOP: '+', '-', '*', '/'
    struct Node *lhs, *rhs;     // N_BINOP: hai vế; N_NEG: lhs; N_ASSIGN: lhs = biểu thức giá trị
} Node;

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
