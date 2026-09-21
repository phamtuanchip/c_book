cmake_minimum_required(VERSION 3.16)
project(geometry_demo VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)                 # -std=c11 thay vì gnu11

# Thư viện tĩnh từ geometry.c
add_library(geometry STATIC src/geometry.c)
target_include_directories(geometry PUBLIC include)
target_compile_options(geometry PRIVATE -Wall -Wextra -Wpedantic)
find_library(MATH_LIB m)                    # libm (trên Windows/MSVC không cần)
if(MATH_LIB)
    target_link_libraries(geometry PUBLIC ${MATH_LIB})
endif()

# Chương trình chính
add_executable(app src/main.c)
target_link_libraries(app PRIVATE geometry)

# Kiểm thử (ctest)
enable_testing()
add_test(NAME smoke COMMAND app)
