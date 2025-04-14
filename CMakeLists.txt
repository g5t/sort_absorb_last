cmake_minimum_required(VERSION 3.30)
project(test-sort C)
set(CMAKE_C_STANDARD 99)

if (${CMAKE_C_COMPILER_ID} STREQUAL "NVHPC")
    message(STATUS "Using NVIDIA HPC SDK compiler")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -O3")
    add_compile_options(-acc=multicore)
else()
    message(STATUS "Using default compiler ${CMAKE_C_COMPILER_ID}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -O3")
endif()

add_executable(tester main.c sort_absorb_last.c particle.c particles.c)

include(CTest)

add_test(NAME test_sort_offsets_soa COMMAND tester 10000 15000 0)
add_test(NAME test_sort_offsets_aos COMMAND tester 10000 15000 1)


add_executable(stats stats.c sort_absorb_last.c particle.c particles.c)