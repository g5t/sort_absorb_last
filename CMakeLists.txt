cmake_minimum_required(VERSION 3.30)
project(test-sort C)
set(CMAKE_C_STANDARD 99)
add_executable(tester main.c sort_absorb_last.c)
