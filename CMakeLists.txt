cmake_minimum_required(VERSION 3.14)

# This is for me rather than good practice as I like clang's error messages
# Could check if env variable CXX is set then use that if so?
# Must be before project line else cmake inifinite loops
# set(CMAKE_CXX_COMPILER "/usr/local/bin/clang++")

project(wordsearch_solver)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 17)
# Prevent madness
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Prevent gnu standards extension stuffs
set(CMAKE_CXX_EXTENSIONS OFF)
# Per target
# target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
# set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

# find_package(fmt CONFIG REQUIRED)
find_package(fmt REQUIRED)
# find_package(spdlog REQUIRED)
find_package(Catch2 REQUIRED)
find_package(Boost 1.70.0 REQUIRED) # COMPONENTS container)
find_package(jr_assert REQUIRED)

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


# Cannot just use set, will end up with CMake list data types
# set(CMAKE_CXX_FLAGS_INIT "-Wall")
# set(CMAKE_CXX_FLAGS_INIT "-Wall -Wextra -pedantic -Wfloat-equal \
# -Wwrite-strings -Wswitch-enum -Wunreachable-code -Wconversion \
# -Wcast-qual -Wstrict-overflow=5 -Werror=shadow -Werror=uninitialized \
# -Werror=return-type -march=native -pipe\
# ")
# message("CMAKE_CXX_FLAGS_INIT ${CMAKE_CXX_FLAGS_INIT}")
# set(COMPILER_FLAGS "hi")
# Changed to warn not error on shadow for now
string(CONCAT COMPILER_FLAGS "-Wall -Wextra -pedantic -Wfloat-equal "
    "-Wwrite-strings -Wswitch-enum -Wunreachable-code -Wconversion "
    "-Wcast-qual -Wstrict-overflow=5 -Werror=uninitialized "
    "-Werror=return-type -pipe")
#  -fno-inline-functions

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-inline-functions ${COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O2 -fno-inline-functions -fno-omit-frame-pointer -march=native ${COMPILER_FLAGS}")
# set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-DNDEBUG -O3 -march=native ${COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -march=native ${COMPILER_FLAGS}")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-fsanitize=address,undefined")
# set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-fsanitize=address,undefined")
# set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-fsanitize=memory -fsanitize-memory-track-origins")
# set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-fsanitize=address,undefined -fsanitize=memory -fsanitize-memory-track-origins")
# set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-fsanitize=address,undefined -fuse-ld=lld")
#set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-fuse-ld=lld")

#set(CMAKE_EXE_LINKER_FLAGS
#  "-fuse-ld=lld -fsanitize=address,undefined ${CMAKE_EXE_LINKER_FLAGS}"
#  )

#target_compile_options(${PROJECT_NAME}_main
#    PRIVATE "-Werror=shadow")
add_library(profiler INTERFACE IMPORTED GLOBAL)
# target_link_options(profiler BEFORE INTERFACE "-Wl,--no-as-needed" "/usr/local/lib/libprofiler.so")
target_link_options(profiler BEFORE INTERFACE "LINKER:--no-as-needed,/usr/local/lib/libprofiler.so")
# Don't believe necessary, more for consistency
add_library(profiler::profiler ALIAS profiler)

#add_library(profiler SHARED IMPORTED GLOBAL)
#set_target_properties(profiler PROPERTIES IMPORTED_LOCATION /usr/local/lib/libprofiler.so)
## Not necesaary but nice for consistency
#add_library(profiler::profiler ALIAS profiler)

# add_library(${PROJECT_NAME} src/wordsearch_solver.cpp include/wordsearch_solver.h)
add_library(${PROJECT_NAME}
  src/dictionary.cpp
  src/stringindexes.cpp
  src/wordsearch_solver.cpp
  src/dictionary_std_set.cpp

  include/dictionary.h
  include/stringindexes.h
  include/wordsearch_solver.h
  include/wordsearch_solver_defs.h
  include/dictionary_std_set.h
  )

# Using this to set files for IDE
# If just add headers using add_library qtcreator complains about ambiguous
# parse something. Although now it isn't?
#set_target_properties(${PROJECT_NAME}
#  PROPERTIES
#  PUBLIC_HEADER "include/wordsearch_solver.h include/stringindexes.h"
#  )
# Needs to be here so that projects using this lib get these flags in their link flags
target_compile_options(${PROJECT_NAME}
    PRIVATE "-Werror=shadow")
# -Weffc++ gives a fair amount of dumb noise errors so disabled
    # PRIVATE "-Werror=shadow" "-Weffc++")
target_link_libraries(${PROJECT_NAME}
    PRIVATE jr_assert::jr_assert
    PUBLIC profiler::profiler -lstdc++fs trie::trie fmt::fmt
  )
#target_link_options(${PROJECT_NAME} BEFORE
#  PRIVATE "-fsanitize=address,undefined")
add_library(wordsearch_solver::wordsearch_solver ALIAS ${PROJECT_NAME})

message("in word_trie_lib PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")

target_include_directories(${PROJECT_NAME}
  PUBLIC ${PROJECT_SOURCE_DIR}/include)
#    PRIVATE ${PROJECT_SOURCE_DIR}/src) # Internal headers?

add_subdirectory(trie)

# Argument parser
add_subdirectory(Lyra)

add_executable(${PROJECT_NAME}_main src/main.cpp)

target_link_libraries(${PROJECT_NAME}_main
  profiler::profiler
  wordsearch_solver::wordsearch_solver
  BFG::Lyra
  jr_assert::jr_assert
  fmt::fmt
  )

#if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#  target_link_options(${PROJECT_NAME}_main BEFORE
#    PRIVATE "-fsanitize=address,undefined")
#endif()

enable_testing()
add_subdirectory(test)

# message("
#     copy ${CMAKE_BINARY_DIR}/compile_commands.json ${PROJECT_SOURCE_DIR}")
add_custom_target(${PROJECT_NAME}_after_all_built ALL)
add_custom_command(TARGET ${PROJECT_NAME}_after_all_built POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E
    copy "${CMAKE_BINARY_DIR}/compile_commands.json" "${PROJECT_SOURCE_DIR}"
	)

# add_custom_command(OUTPUT "non"
# 	COMMAND "${CMAKE_COMMAND}" -E
#     copy "${CMAKE_BINARY_DIR}/compile_commands.json" "${PROJECT_SOURCE_DIR}"
#     DEPENDS dasf
#     # DEPENDS "${PROJECT_NAME}" "${PROJECT_NAME}_main"
#     )

# target_compile_features(word_trie_lib PRIVATE cxx_std_17)
# set_target_properties(word_trie_lib PROPERTIES CXX_EXTENSIONS OFF)
# target_compile_options(word_trie_lib PRIVATE "${COMPILER_FLAGS}")
# target_sources(word_trie_lib PUBLIC word_trie_lib.h)
# target_include_directories(word_trie_lib PRIVATE ${CMAKE_CURRENT_LIST_DIR})
# target_include_directories(word_trie_lib PUBLIC src)

# install(
#     TARGETS word_trie_lib
#     EXPORT word_trie_lib-config
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# install(
# 	EXPORT word_trie_lib-config
# 	NAMESPACE word_trie_lib::
# 	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/word_trie_lib)

# install(
# 	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include
# 	DESTINATION ${CMAKE_INSTALL_LIBDIR}/word_trie_lib)


# add_executable(main main.cpp)
# target_compile_features(main PRIVATE cxx_std_17)
# set_target_properties(main PROPERTIES CXX_EXTENSIONS OFF)
# target_compile_options(main PRIVATE "${COMPILER_FLAGS}")

# target_link_libraries(main word_trie_lib fmt::fmt)


# add_library(word_trie_lib SHARED word_trie_lib.cpp)
# target_compile_features(word_trie_lib PRIVATE cxx_std_17)
# set_target_properties(word_trie_lib PROPERTIES CXX_EXTENSIONS OFF)
# target_compile_options(word_trie_lib PRIVATE "${COMPILER_FLAGS}")
# target_sources(word_trie_lib PUBLIC word_trie_lib.h)

# set_target_properties(word_trie_lib PROPERTIES PUBLIC_HEADER word_trie_lib.h)


# add_subdirectory(src)
# add_subdirectory(test)
# enable_testing()
# catch_discover_tests(test)

