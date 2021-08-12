cmake_minimum_required(VERSION 3.19)

project(bench)
# set(CMAKE_EXECUTABLE_SUFFIX ".html")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

find_package(benchmark REQUIRED)

add_executable(bench bench.cpp)
target_link_libraries(bench PRIVATE
    link_std_filesystem::link_std_filesystem
    benchmark::benchmark wordsearch_solver::wordsearch_solver)
set_target_properties(bench PROPERTIES
    CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

