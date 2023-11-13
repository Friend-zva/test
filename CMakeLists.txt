cmake_minimum_required(VERSION 3.7)
project(tests LANGUAGES C)

set(CMAKE_C_STANDARD 99)

file(GLOB_RECURSE MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c)

add_executable(main ${MAIN_SRC})
target_compile_options(main PRIVATE )
target_link_options(main PRIVATE )
