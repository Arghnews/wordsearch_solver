cmake_minimum_required(VERSION 3.19)

project(cmdline_app)
# set(CMAKE_EXECUTABLE_SUFFIX ".html")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

find_package(cxxopts REQUIRED)
find_package(fmt REQUIRED)
find_package(gperftools REQUIRED)
find_package(range-v3 REQUIRED)

# add_library(profiler INTERFACE IMPORTED GLOBAL)
# target_link_options(profiler BEFORE INTERFACE "LINKER:--no-as-needed,/usr/local/lib/libprofiler.a" "LINKER:-lunwind")
# # target_sources()
# # Don't believe necessary, more for consistency
# add_library(profiler::profiler ALIAS profiler)

add_executable(cmdline_app main.cpp)
target_link_libraries(cmdline_app PUBLIC
    cxxopts::cxxopts
    wordsearch_solver::wordsearch_solver
    fmt::fmt
    gperftools::gperftools
    range-v3::range-v3
    )
set_target_properties(cmdline_app PROPERTIES
    CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
