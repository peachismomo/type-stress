cmake_minimum_required(VERSION 3.20)

add_executable(stress_example example.cpp)
target_link_libraries(stress_example PRIVATE stress)
