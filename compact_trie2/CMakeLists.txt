cmake_minimum_required(VERSION 3.14)

project(compact_trie2)

find_package(fmt REQUIRED)
find_package(range-v3 REQUIRED)

# Project wide cpp standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(HEADERS
    "compact_trie2.hpp"
    "compact_trie2_iterator_typedefs.hpp"
    "compact_trie2.tpp"
    "empty_node_view.hpp"
    "flat_char_value_map.hpp"
    "full_node_view.hpp"
    "mini_offsets.hpp"
    )

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
list(TRANSFORM HEADERS PREPEND "${INCLUDE_DIR}/")

add_library(${PROJECT_NAME}
    ${HEADERS}
    "src/compact_trie2.cpp"
    "src/empty_node_view.cpp"
    "src/full_node_view.cpp"
    )
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    fmt::fmt
    range-v3::range-v3
    )

get_directory_property(hasParent PARENT_DIRECTORY)
# If this is being built as top level project
if(NOT hasParent)
    include(main.CMakeLists.txt)
endif()

