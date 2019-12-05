cmake_minimum_required(VERSION 3.14)
project(trie_wrapper)

find_package(fmt REQUIRED)
find_package(trie REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE
    ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
    trie::trie
    jr_assert::jr_assert
    fmt::fmt
    )

# TODO: add CMake requires compiler features/version etc.
