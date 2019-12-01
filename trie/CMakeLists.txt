cmake_minimum_required(VERSION 3.14)
project(trie)

find_package(fmt REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)
add_library(trie::trie ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE
    ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
    jr_assert::jr_assert
    fmt::fmt
    )

# TODO: add CMake requires compiler features/version etc.
