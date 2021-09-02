cmake_minimum_required(VERSION 3.19)

project(wordsearch_solver)
# set(CMAKE_EXECUTABLE_SUFFIX ".html")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Project wide cpp standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# We need to pass -stdc++fs as a link flag here. We could do this in conan
# in package_info with self.cpp_info.libs += ["-lstdc++fs"]
# However that means that consumers of this from cmake won't get the flag
# so we set it here instead. This assumes libstdc++.
# Problem with putting this in target_link_libraries:
# $<IF:$<AND:$<CXX_COMPILER_ID:Clang>,$<PLATFORM_ID:Linux>>,"stdc++fs","">
# If you're the IF is false, cmake then helpfully appends -l"" to your link options
    # $<:$<AND:$<CXX_COMPILER_ID:Clang>,$<PLATFORM_ID:Linux>>,"stdc++fs">
    # $<IF:$<AND:$<CXX_COMPILER_ID:Clang>,$<PLATFORM_ID:Linux>>,"stdc++fs","">
# Too fiddly, moved into "library" here
add_library(link_std_filesystem INTERFACE IMPORTED)
add_library(link_std_filesystem::link_std_filesystem ALIAS link_std_filesystem)
target_link_libraries(link_std_filesystem INTERFACE
    $<$<AND:$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>,$<PLATFORM_ID:Linux>>:stdc++fs>)

# https://blog.kitware.com/cmake-and-the-default-build-type/
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(default_build_type "DEBUG")
    message(STATUS "Setting build type to '${default_build_type}' as none specified")
    # I prefer having it remind you you've forgotten to set the build type every time
    set(CMAKE_BUILD_TYPE "${default_build_type}")# CACHE
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


string(JOIN " " COMPILER_FLAGS "-Wall -Wextra -pedantic -Wfloat-equal"
"-Wwrite-strings -Wswitch-enum -Wunreachable-code -Wconversion"
"-Wcast-qual -Wstrict-overflow=5 -Werror=uninitialized"
"-Werror=return-type -pipe -fdiagnostics-color")

# string(APPEND CMAKE_CXX_FLAGS " ${COMPILER_FLAGS} ")
string(APPEND CMAKE_CXX_FLAGS " ${COMPILER_FLAGS} ")

# Don't want to build with sanitizers when installing with cmake or packaging,
# otherwise downstream users MUST also enable sanitizers

# This increases link time considerably, undesirable in debug builds
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(DEVELOPER_FLAGS FALSE CACHE BOOL "Enable warning/sanitizer flags during development of library")
set(USE_LINKER_FLAGS "")
set(LTO "")
if (DEVELOPER_FLAGS)
    set(SANITISERS " -fsanitize=address,undefined ")
endif()

# Who doesn't have these?
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    string(APPEND USE_LINKER_FLAGS "-fuse-ld=lld")
    set(LTO "-flto=thin")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    string(APPEND USE_LINKER_FLAGS "-fuse-ld=gold")
    set(LTO "-flto -fno-fat-lto-objects")
endif()


string(APPEND CMAKE_CXX_FLAGS_DEBUG " -g -O0 -fno-omit-frame-pointer ${SANITISERS} ")
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " -g -O2 -march=native -fno-omit-frame-pointer ${SANITISERS} ${LTO}")
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -g -O3 -march=native -DNDEBUG ${LTO}")

# string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG " -flto -fno-omit-frame-pointer ${SANITISERS} -fuse-ld=lld ")
# string(APPEND CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO " -flto -fno-omit-frame-pointer ${SANITISERS} -fuse-ld=lld ")
# string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " -flto -fuse-ld=lld ")
string(APPEND CMAKE_EXE_LINKER_FLAGS " ${USE_LINKER_FLAGS}")
string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG " -fno-omit-frame-pointer ${SANITISERS}")
string(APPEND CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO " -fno-omit-frame-pointer ${SANITISERS} ${LTO}")
string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE "${LTO}")

string(APPEND CMAKE_SHARED_LINKER_FLAGS "${USE_LINKER_FLAGS}")

# Usage:
# make_compile_commands(
    # TARGET_TO_REBUILD_ON "${PROJECT_NAME}"
    # DESTINATION "${PROJECT_SOURCE_DIR}")
function(make_compile_commands)
    set(oneValueArgs DESTINATION TARGET_TO_REBUILD_ON)
    cmake_parse_arguments(MAKE_COMPILE_COMMANDS "" "${oneValueArgs}" "" ${ARGN})
    set(DEP "${MAKE_COMPILE_COMMANDS_TARGET_TO_REBUILD_ON}_compile_commands_json")
    add_custom_target("${DEP}" ALL VERBATIM)
    add_custom_command(TARGET "${DEP}" POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
        copy "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${MAKE_COMPILE_COMMANDS_DESTINATION}"
        VERBATIM
        )
    add_dependencies("${MAKE_COMPILE_COMMANDS_TARGET_TO_REBUILD_ON}" "${DEP}")
endfunction()

add_custom_target(${PROJECT_NAME}_after_all_built ALL VERBATIM)
add_custom_command(TARGET ${PROJECT_NAME}_after_all_built POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E
    copy "${CMAKE_BINARY_DIR}/compile_commands.json" "${CMAKE_SOURCE_DIR}"
    VERBATIM
    )


list(APPEND ALL_SOLVERS trie compact_trie compact_trie2 dictionary_std_vector dictionary_std_set)
list(APPEND ALL_SOLVERS_CLASSNAMES Trie CompactTrie CompactTrie2 DictionaryStdVector DictionaryStdSet)
set(WORDSEARCH_SOLVERS "${ALL_SOLVERS}" CACHE STRING "Semicolon separated string of which solvers to build, from ${ALL_SOLVERS}")

# Cmake list variable of the form "trie::trie compact_trie::compact_trie " etc.
# These are the solver dict implementation cmake library targets to link against
list(APPEND ALL_DICTIONARIES_CMAKE_LINK_LIBRARIES)

# This list is JOINed into a string of the form, "trie::Trie, compact_trie::CompactTrie"
# This can be used in c++ code as the basis for building a compile time list of
# solver dict implementations
list(APPEND ALL_DICTIONARY_CLASSES)

# This list is JOINed with newline characters to form a variable that has all
# the required solver dict implementation includes
# ie.
# #include "wordsearch_solver/trie/trie.hpp"
# #include "wordsearch_solver/compact_trie/compact_trie.hpp"
# etc.
list(APPEND ALL_SOLVER_INCLUDES)

macro(add_component snake_case_name _pascal_case_name)
    list(APPEND ALL_DICTIONARIES_CMAKE_LINK_LIBRARIES "${snake_case_name}::${snake_case_name}")
    message(DEBUG "Adding ${_pascal_case_name}")
    list(APPEND ALL_DICTIONARY_CLASSES "${snake_case_name}::${_pascal_case_name}")
    add_subdirectory(${snake_case_name})
    list(APPEND ALL_SOLVER_INCLUDES
        "#include \"${PROJECT_NAME}/${snake_case_name}/${snake_case_name}.hpp\"")
endmacro()

add_subdirectory(utility)

if ("" STREQUAL "${WORDSEARCH_SOLVERS}")
    message(FATAL_ERROR "Solvers specified in WORDSEARCH_SOLVERS may not be empty, must be semicolon separated list of items from ${ALL_SOLVERS}")
endif()

# This variable is JOINed with newlines into a string, of the form
# #define WORDSEARCH_SOLVER_HAS_TRIE
# #define WORDSEARCH_SOLVER_HAS_COMPACT_TRIE
# etc. for all the solver dicts that have been built
list(APPEND SOLVER_C_DEFINES)

# Used in config.hpp.in as well as here
string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPERCASE)

foreach(SOLVER IN LISTS WORDSEARCH_SOLVERS)
    if (NOT (SOLVER IN_LIST ALL_SOLVERS))
        message(FATAL_ERROR "Unrecognised solver \"${SOLVER}\", should be one of ${ALL_SOLVERS}")
    else()
        message(DEBUG "Found solver ${SOLVER}")
        list(FIND ALL_SOLVERS "${SOLVER}" index)
        message(DEBUG "Adding index ${index}")
        list(GET ALL_SOLVERS_CLASSNAMES ${index} pascal_case_name)
        message(DEBUG "Adding got solver classname ${pascal_case_name}")
        add_component("${SOLVER}" "${pascal_case_name}")
        list(APPEND SOLVER_C_DEFINES "#define ${PROJECT_NAME_UPPERCASE}_HAS_${SOLVER}")
    endif()
endforeach()
list(JOIN ALL_DICTIONARY_CLASSES ", " ALL_DICTIONARY_CLASSES)
list(JOIN ALL_SOLVER_INCLUDES "\n" ALL_SOLVER_INCLUDES)
list(JOIN SOLVER_C_DEFINES "\n" SOLVER_C_DEFINES)

string(JOIN "\n" config_file_content
    "#ifndef ${PROJECT_NAME_UPPERCASE}_CONFIG_HPP"
    "#define ${PROJECT_NAME_UPPERCASE}_CONFIG_HPP"
    "#define WORDSEARCH_DICTIONARY_CLASSES ${ALL_DICTIONARY_CLASSES}"
    "${ALL_SOLVER_INCLUDES}"
    "${SOLVER_C_DEFINES}"
    "#endif // ${PROJECT_NAME_UPPERCASE}_CONFIG_HPP"
    )


# Build config as a tiny library so can neatly depend on it in solver/CMakeLists.txt so it gets put in the include path correctly

# Or just add it to the include dirs in solver somehow but seems unneat

include(GNUInstallDirs)

set(PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")
set(PROJECT_INCLUDE_PROJECT_DIR "${PROJECT_INCLUDE_DIR}/${PROJECT_NAME}")


# Generate project master header file to be included
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}.hpp.in"
    "${PROJECT_INCLUDE_PROJECT_DIR}/${PROJECT_NAME}.hpp"
    @ONLY)

# Generate config.hpp, contains project public macros inside
file(CONFIGURE
    OUTPUT "${PROJECT_INCLUDE_PROJECT_DIR}/config.hpp"
    CONTENT "${config_file_content}")

# The solver subproject needs these headers that are generated here, so it
# links against/depends on config::config
set(config_headers
    "${PROJECT_INCLUDE_PROJECT_DIR}/${PROJECT_NAME}.hpp"
    "${PROJECT_INCLUDE_PROJECT_DIR}/config.hpp")
add_library(config INTERFACE ${config_headers})
set_target_properties(config PROPERTIES PUBLIC_HEADER "${config_headers}")
add_library(config::config ALIAS config)

target_include_directories(config INTERFACE
    $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
install(TARGETS config EXPORT config-targets
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
install(EXPORT config-targets FILE config-targets.cmake
    NAMESPACE config::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    )


configure_file(test/test_cases/massive_wordsearch.txt "${CMAKE_CURRENT_BINARY_DIR}" COPYONLY)

add_subdirectory(solver)

add_subdirectory(benchmark)

# This takes bloody ages to link, so when not needed, don't build it
set(WORDSEARCH_SOLVER_BUILD_CMDLINE_APP ON CACHE BOOL "Build the cmdline_app")
if (WORDSEARCH_SOLVER_BUILD_CMDLINE_APP)
    add_subdirectory(cmdline_app)
endif()

add_subdirectory(example)

# Master/public library
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
target_link_libraries(${PROJECT_NAME} INTERFACE
    config::config
    solver::solver
    utility::utility
    ${ALL_DICTIONARIES_CMAKE_LINK_LIBRARIES}
    )


# https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html
include(CMakePackageConfigHelpers)


install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")

# Generate a targets file and install it
install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
         )

include(version.cmake)
# Defines ${PROJECT_NAME}_(MAJOR|MINOR|PATCH) and ${PROJECT_NAME}_VERSION_STRING

####################################
# https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html

# Generates a package version file in the build directory
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    VERSION "${WORDSEARCH_SOLVER_VERSION_STRING}"
    COMPATIBILITY AnyNewerVersion
    )

# Install the package version file to the install directory
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

####################################

# Generate a cmake package config file
configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

# Install the package config file
install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
          DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)


# Install the project master header file that should be included,
# and any other headers required
# install(FILES
    # "${PROJECT_GENERATED_HEADERS_DIR}/config.hpp"
    # "${PROJECT_GENERATED_HEADERS_DIR}/${PROJECT_NAME}.hpp"
    # DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")

# Disabled for now because if we run in webpage we run out of memory
# If we run in node we get
# /home/justin/cpp/ws3/build/bin/the_test.js:40
# throw 'using preloaded data can only be done on a web page or in a web worker';
enable_testing()
add_subdirectory(test)

