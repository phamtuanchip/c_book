Chapter 1 code samples

Files:
- hello.c: Hello World example
- hello_name.c: Prompt for name and greet the user
- arithmetic.c: Read two integers and print sum, diff, product, quotient
- input_validation.c: Safe integer input using fgets + strtol
- Makefile: build targets

Build (Linux/macOS, MSYS2):
- make

Or build manually:
- gcc -o hello hello.c
- gcc -o hello_name hello_name.c
- gcc -o arithmetic arithmetic.c
- gcc -o input_validation input_validation.c

Run:
- ./hello
- ./hello_name
- ./arithmetic
- ./input_validation

Notes:
- Use -Wall -Wextra when compiling to see warnings.
- On Windows with MinGW, executables have .exe extension.
