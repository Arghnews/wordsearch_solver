cmake_minimum_required(VERSION 3.14)

project(trie)

find_package(fmt REQUIRED)
find_package(range-v3 REQUIRED)

# Project wide cpp standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
    # message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

set(HEADERS
    "trie.hpp"
    "trie.tpp"
    "node.hpp"
    )

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
list(TRANSFORM HEADERS PREPEND "${INCLUDE_DIR}/")

add_library(${PROJECT_NAME}
    ${HEADERS}
    "src/trie.cpp"
    "src/node.cpp"
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

