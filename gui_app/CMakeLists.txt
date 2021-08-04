cmake_minimum_required(VERSION 3.18)

project(test_project)

# Enable use with conan generator cmake_find_package
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

find_package(fmt REQUIRED)
find_package(imgui_my REQUIRED)
find_package(wordsearch_solver REQUIRED)
find_package(gperftools REQUIRED)

# find_package(fmt REQUIRED)
# find_package(range-v3 REQUIRED)
# find_package(span-lite REQUIRED)
# find_package(Boost QUIET REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Project wide cpp standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

string(JOIN " " COMPILER_FLAGS "-Wall -Wextra -pedantic -Wfloat-equal"
"-Wwrite-strings -Wswitch-enum -Wunreachable-code -Wconversion"
"-Wcast-qual -Wstrict-overflow=5 -Werror=uninitialized"
"-Werror=return-type -pipe -fdiagnostics-color")

string(APPEND CMAKE_CXX_FLAGS " ${COMPILER_FLAGS} ")

set(SANITISERS " -fsanitize=address,undefined ")
# set(SANITISERS "")
string(APPEND CMAKE_CXX_FLAGS_DEBUG " -g -O0 -fno-omit-frame-pointer ${SANITISERS} ")
# Enable asserts in RelWithDebInfo build type
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " -g -O2 -march=native -flto -fno-omit-frame-pointer ${SANITISERS} ")
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -g -O3 -march=native -flto -DNDEBUG ")

string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG " -flto -fno-omit-frame-pointer ${SANITISERS} -fuse-ld=lld ")
string(APPEND CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO " -flto -fno-omit-frame-pointer ${SANITISERS} -fuse-ld=lld ")
string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " -flto -fuse-ld=lld ")

add_custom_target(${PROJECT_NAME}_after_all_built ALL VERBATIM)
add_custom_command(TARGET ${PROJECT_NAME}_after_all_built POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E
    copy "${CMAKE_BINARY_DIR}/compile_commands.json" "${CMAKE_SOURCE_DIR}"
    VERBATIM
    )

add_executable(main main.cpp)
target_link_libraries(main PRIVATE fmt::fmt imgui_my::imgui_my wordsearch_solver::wordsearch_solver gperftools::gperftools)
    # nonstd::span-lite fmt::fmt range-v3::range-v3 Boost::Boost)
set_target_properties(main PROPERTIES
    CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/dictionary.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/dictionary.txt" COPYONLY)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/wordsearch.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/wordsearch.txt" COPYONLY)

# Copy font files -> share/fonts/*.ttf
# file(COPY "${imgui_my_RES_DIRS}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")

