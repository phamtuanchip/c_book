#ifndef MATH_H
#define MATH_H

#include <stddef.h>

/* Simple math library API */
int add_int(int a, int b);
int sub_int(int a, int b);
int mul_int(int a, int b);
/* divide returns 0 on success and writes result to out; returns non-zero on divide by zero */
int div_int(int a, int b, int *out);

#endif /* MATH_H */
