cmake_minimum_required(VERSION 3.10)
project(chapter11_sample C)

add_library(mymodule STATIC module.c)
add_executable(ch11_main main.c)

target_include_directories(ch11_main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ch11_main PRIVATE mymodule)
