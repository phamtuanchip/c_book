// calc_table.c
#include <stdio.h>

typedef double (*BinOp)(double, double);

static double op_add(double a, double b) { return a + b; }
static double op_sub(double a, double b) { return a - b; }
static double op_mul(double a, double b) { return a * b; }
static double op_div(double a, double b) { return b != 0 ? a / b : 0; }

static const struct { char sym; BinOp fn; } TABLE[] = {
    { '+', op_add }, { '-', op_sub }, { '*', op_mul }, { '/', op_div },
};

int main(void) {
    double a, b;
    char op;
    if (scanf("%lf %c %lf", &a, &op, &b) != 3) { fprintf(stderr, "cu phap: so toan_tu so\n"); return 1; }

    for (size_t i = 0; i < sizeof TABLE / sizeof TABLE[0]; i++) {
        if (TABLE[i].sym == op) {
            if (op == '/' && b == 0) { fprintf(stderr, "chia cho 0\n"); return 1; }
            printf("= %g\n", TABLE[i].fn(a, b));
            return 0;
        }
    }
    fprintf(stderr, "toan tu khong ho tro: %c\n", op);
    return 1;
}
