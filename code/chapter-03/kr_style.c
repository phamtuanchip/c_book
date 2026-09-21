/* K&R-style function declaration example (historical)
 * This example demonstrates old K&R function declaration syntax for teaching only.
 * Modern compilers may require -std=gnu89 or similar flags to accept this form.
 */
#include <stdio.h>

/* K&R style: parameter types declared separately (historical) */
int add(a, b)
int a;
int b;
{
    return a + b;
}

int main(void) {
    printf("K&R add(2,3) = %d\n", add(2,3));
    return 0;
}
