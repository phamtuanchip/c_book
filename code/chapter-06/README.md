Chapter 6 code samples

Files:
- math.h, math.c: simple math library (add, sub, mul, div with error check)
- main.c: example using the library and demonstrating static local variable
- Makefile: build library and example

Build:
- gcc -Wall -Wextra -std=c11 -c math.c -o math.o
- gcc -Wall -Wextra -std=c11 -c main.c -o main.o
- gcc -o demo_math main.o math.o

Or use Makefile: make

Run:
- ./demo_math
