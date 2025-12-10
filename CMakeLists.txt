cmake_minimum_required(VERSION 3.20)
project(type-stress LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(STRESS_BUILD_TESTS "Build the stress test suite" OFF)


add_subdirectory(stress)
add_subdirectory(example)
if(STRESS_BUILD_TESTS)
    include(CTest)
    add_subdirectory(tests)
endif()

