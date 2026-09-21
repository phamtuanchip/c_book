Chapter 11 sample module and build examples

Files:
- module.h / module.c: simple module API and implementation
- main.c: uses the module
- Makefile: build with gcc
- CMakeLists.txt: build with CMake

Build with Make:
- make
- ./main

Build with CMake:
- mkdir build && cd build
- cmake ..
- cmake --build .
- ./ch11_main
