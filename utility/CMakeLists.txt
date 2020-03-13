cmake_minimum_required(VERSION 3.14)

if (TARGET utility)
    return()
endif()

project(utility CXX)

if (NOT TARGET fmt)
	find_package(fmt REQUIRED)
endif()
if (NOT TARGET range-v3)
	find_package(range-v3 REQUIRED)
endif()

# Project wide cpp standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(HEADERS
    "utility/utility.hpp"
    "utility/flat_char_value_map.hpp"
    )

# set(SOURCES
    # )

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

list(TRANSFORM HEADERS PREPEND "${INCLUDE_DIR}/")
# list(TRANSFORM SOURCES PREPEND "${SOURCE_DIR}/")

add_library(${PROJECT_NAME}
    ${HEADERS}
    # ${SOURCES}
    )
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${PROJECT_NAME} INTERFACE ${INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME}
	INTERFACE
    fmt::fmt
    range-v3::range-v3
    )
