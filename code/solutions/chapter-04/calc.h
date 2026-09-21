// calc.h
#ifndef CALC_H
#define CALC_H

int calc_add(int a, int b);
int calc_sub(int a, int b);
int calc_mul(int a, int b);
/* Trả 0 và ghi kết quả vào *out; trả -1 nếu b == 0. */
int calc_div_safe(int a, int b, int *out);

#endif
