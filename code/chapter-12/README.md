Chapter 12 code samples

Files:
- copy_file.c: copy binary files using fread/fwrite
- csv_parser.c: simple CSV parser (no quoted fields)

Build:
- gcc -Wall -Wextra -std=c11 -o copy_file copy_file.c
- gcc -Wall -Wextra -std=c11 -o csv_parser csv_parser.c

Run:
- ./copy_file source.bin dest.bin
- ./csv_parser
