# Set the project name
cmake_minimum_required (VERSION 3.14)
project (word_trie)

# Must use clang for magic_enum to work (unless gcc 9)
SET (CMAKE_CXX_COMPILER "clang++")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
# message ("${PROJECT_NAME}")

find_package (fmt CONFIG REQUIRED)
find_package (spdlog REQUIRED)
# find_package (Boost 1.70.0 REQUIRED COMPONENTS container)

add_executable(${PROJECT_NAME} main.cpp)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_compile_options (${PROJECT_NAME} PRIVATE -g -Wall -Wextra -pedantic -Wfloat-equal -Wwrite-strings -Wswitch-enum -Wunreachable-code -Wconversion -Wcast-qual -Wstrict-overflow=5 -Werror=shadow -march=native -pipe -fsanitize=undefined -Werror=uninitialized -Werror=return-type -pg -O2)

# add_definitions (-DOLD_SURROUNDING)
# set_target_properties (${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "OLD_SURROUNDING")

# Seems to have worked well for header only lib
# magic enum NEEDS either clang or gcc >= 9
add_library (magic_enum INTERFACE)
target_include_directories (magic_enum INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/magic_enum/include)
# This line is supposed to help with telling IDEs and therefore I'd of assumed the compile_commands.json about this, however seems to work without it
# target_sources (magic_enum INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/magic_enum/include/magic_enum.hpp)

# message (${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries (${PROJECT_NAME} PRIVATE fmt::fmt spdlog::spdlog magic_enum "-fsanitize=undefined" -lstdc++fs -pg)

add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E
	copy "${CMAKE_BINARY_DIR}/compile_commands.json" "${CMAKE_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}"
	)

# add_subdirectory(magic_enum)

# # Header only

# add_library(${PROJECT_NAME} INTERFACE)
# add_library(${CMAKE_PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# target_include_directories(${PROJECT_NAME}
#     INTERFACE
#         ${PROJECT_SOURCE_DIR}/include
# )
