cmake_minimum_required(VERSION 3.30)
project(test-sort C)
set(CMAKE_C_STANDARD 99)
add_executable(tester main.c sort_absorb_last.c particle.c)

include(CTest)

foreach(i RANGE 1 10)
    add_test(NAME test_sort_offsets_${i} COMMAND tester ${i} 0)
endforeach ()
foreach(i RANGE 20 100 10)
    add_test(NAME test_sort_offsets_${i} COMMAND tester ${i} 0)
endforeach ()
foreach(i RANGE 200 1000 100)
    add_test(NAME test_sort_offsets_${i} COMMAND tester ${i} 0)
endforeach ()
foreach(i RANGE 2000 10000 1000)
    add_test(NAME test_sort_offsets_${i} COMMAND tester ${i} 0)
endforeach ()


#foreach(i RANGE 1 10)
#    add_test(NAME test_sort_pointers_${i} COMMAND tester ${i} 1)
#endforeach ()