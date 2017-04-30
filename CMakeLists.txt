cmake_minimum_required (VERSION 3.1)
project (eytzinger)

include_directories (fixed_eytzinger_map/include)
include_directories (external/Catch/include)

add_executable(eytzinger fixed_eytzinger_map/tests/fixed_eytzinger_map_sanity_tests.cpp)

enable_testing()
add_test(NAME Test COMMAND eytzinger)
