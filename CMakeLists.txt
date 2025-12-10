cmake_minimum_required(VERSION 3.20)
project(type-stress LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)

add_subdirectory(stress)
add_subdirectory(example)
add_subdirectory(tests)

