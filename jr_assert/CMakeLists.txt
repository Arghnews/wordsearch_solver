cmake_minimum_required(VERSION 3.14)
project(jr_assert)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)
add_library(jr_assert::jr_assert ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE
    ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
    fmt::fmt
    spdlog::spdlog
    )

# TODO: add CMake requires compiler features/version etc.
