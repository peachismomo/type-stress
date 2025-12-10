cmake_minimum_required(VERSION 3.20)

add_library(stress INTERFACE)

target_include_directories(stress INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
