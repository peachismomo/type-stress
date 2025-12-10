cmake_minimum_required(VERSION 3.20)

include(${CMAKE_SOURCE_DIR}/cmake/Catch2.cmake)

add_executable(stress_tests test_reflection.cpp)
target_link_libraries(stress_tests PRIVATE stress_reflection Catch2::Catch2WithMain)
add_test(NAME reflection_tests COMMAND stress_tests)

