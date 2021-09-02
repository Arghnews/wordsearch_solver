cmake_minimum_required(VERSION 3.19)

project(example)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

find_package(fmt REQUIRED)

add_executable(example example.cpp)
target_link_libraries(example PUBLIC
    wordsearch_solver::wordsearch_solver
    fmt::fmt
    )
set_target_properties(example PROPERTIES
    CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
