# cmake_minimum_required(VERSION <specify CMake version here>)
project(list2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c11 -ansi -static-libgcc ")

set(SOURCE_FILES main.c lisp_mu.c tinyprintf.c)
add_executable(list2 ${SOURCE_FILES})

set(TEST_FILES test_all.c lisp_mu.c tinyprintf.c)
add_executable(lisp_mu_test ${TEST_FILES})
