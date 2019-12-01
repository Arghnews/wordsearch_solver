cmake_minimum_required(VERSION 3.14)

project(trie_main)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(fmt REQUIRED)
# find_package(spdlog REQUIRED)

# https://blog.kitware.com/cmake-and-the-default-build-type/
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(default_build_type "DEBUG")
    message(STATUS "Setting build type to '${default_build_type}' as none was specified")
    # I prefer having it remind you you've forgotten to set the build type every time
    set(CMAKE_BUILD_TYPE "${default_build_type}")# CACHE
        # STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


string(CONCAT COMPILER_FLAGS "-Wall -Wextra -pedantic -Wfloat-equal "
    "-Wwrite-strings -Wswitch-enum -Wunreachable-code -Wconversion "
    "-Wcast-qual -Wstrict-overflow=5 -Werror=uninitialized "
    "-Werror=return-type -pipe")

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 ${COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O2 -fno-omit-frame-pointer -march=native ${COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -march=native ${COMPILER_FLAGS}")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-fsanitize=address,undefined")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-fsanitize=address,undefined")

add_subdirectory(jr_assert)

add_subdirectory(trie)

add_executable(${PROJECT_NAME} main.cpp)

# How to import a shared library just as the "object"
add_library(pro INTERFACE IMPORTED GLOBAL)
# set_target_properties(pro PROPERTIES
    # IMPORTED_LOCATION /usr/local/lib/libprofiler.so
    # )
    # target_link_options(pro BEFORE INTERFACE LINKER:-Wl,--no-as-needed "/usr/local/lib/libprofiler.so")
target_link_options(pro BEFORE INTERFACE "-Wl,--no-as-needed" "/usr/local/lib/libprofiler.so")
# target_link_options(pro BEFORE INTERFACE "-Wl,--no-as-needed" "/usr/local/lib/libprofiler.so" "-Wl,--as-needed")
# target_link_libraries(pro INTERFACE -lprofiler)
add_library(pro::pro ALIAS pro)
# find_library(pro NAMES libprofiler.so PATHS /usr/local/lib/)
# target_link_libraries(pro INTERFACE -lprofiler)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  pro::pro
  trie::trie
  # pro
  jr_assert::jr_assert
  fmt::fmt
  # spdlog::spdlog
  # -lprofiler
  )

target_link_options(${PROJECT_NAME} BEFORE
  PRIVATE "-fsanitize=address,undefined")

add_custom_target(after_all_built ALL)
add_custom_command(TARGET after_all_built POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E
    copy "${CMAKE_BINARY_DIR}/compile_commands.json" "${PROJECT_SOURCE_DIR}"
	)

