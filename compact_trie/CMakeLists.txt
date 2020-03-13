cmake_minimum_required(VERSION 3.14)

project(compact_trie)

find_package(fmt REQUIRED)
find_package(range-v3 REQUIRED)

# Project wide cpp standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(HEADERS
    "compact_trie.hpp"
    "compact_trie.tpp"
    # "flat_char_value_map.hpp"
    "node.hpp"
    )

set(SOURCES
    "node.cpp"
    "compact_trie.cpp"
    )

list(TRANSFORM HEADERS PREPEND "${INCLUDE_DIR}/")
list(TRANSFORM SOURCES PREPEND "${SRC_DIR}/")

add_library(${PROJECT_NAME}
    ${HEADERS}
    ${SOURCES}
    )
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

add_subdirectory(../utility ${CMAKE_CURRENT_BINARY_DIR}/utility)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    fmt::fmt
    range-v3::range-v3
    utility::utility
    )

get_directory_property(hasParent PARENT_DIRECTORY)
# If this is being built as top level project
if(NOT hasParent)
    include(main.CMakeLists.txt)
endif()

