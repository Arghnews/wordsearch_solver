cmake_minimum_required(VERSION 3.18)

project(wordsearch_solver)
# set(CMAKE_EXECUTABLE_SUFFIX ".html")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

# set(INSTALL_INCLUDE_DIR "include")

# if(CONAN_EXPORTED) # in conan local cache
# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
# conan_basic_setup()
# else() # in user space
    # if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    # file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
    # endif()
    # include(${CMAKE_BINARY_DIR}/conan.cmake)
    # conan_cmake_run(CONANFILE conanfile.py BASIC_SETUP CMAKE_TARGETS BUILD missing)
# endif()


# if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   # message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   # file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
                 # "${CMAKE_BINARY_DIR}/conan.cmake")
# endif()
# include(${CMAKE_BINARY_DIR}/conan.cmake)


# if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   # message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   # file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
                 # "${CMAKE_BINARY_DIR}/conan.cmake")
# endif()
# include(${CMAKE_BINARY_DIR}/conan.cmake)

# conan_cmake_run(REQUIRES
    # solver_utility/0.1
    # # fmt/[>=6.0] range-v3/[>=0.10.0] solver_utility/0.1
    # # boost_container/[>=1.69.0]@bincrafters/stable
    # # google-profiler/0.1
                # BASIC_SETUP CMAKE_TARGETS
                # BUILD missing)

# SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed")


# TODO: remove this
# set(BUILD_SHARED_LIBS TRUE)
# set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)


# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
# conan_basic_setup()
# set(RULE_MESSAGES OFF)
# set(CMAKE_INSTALL_MESSAGE NEVER)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_VERBOSE_MAKEFILE ON)

# Project wide cpp standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# https://blog.kitware.com/cmake-and-the-default-build-type/
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(default_build_type "DEBUG")
    message(STATUS "Setting build type to '${default_build_type}' as none specified")
    # I prefer having it remind you you've forgotten to set the build type every time
    set(CMAKE_BUILD_TYPE "${default_build_type}")# CACHE
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


# if(CONAN_EXPORTED) # in conan local cache
    # if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
        # file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
    # endif()
    # include(${CMAKE_BINARY_DIR}/conan.cmake)
    # conan_cmake_run(REQUIRES fmt/[>=6.0] range-v3/[>=0.10.0] solver_utility/0.1
                    # BASIC_SETUP CMAKE_TARGETS
                    # BUILD missing)
# endif()


# Google profiler, assuming can use this in "add_subdirectory" projects
# add_library(profiler INTERFACE IMPORTED GLOBAL)
# target_link_options(profiler BEFORE INTERFACE "LINKER:--no-as-needed,/usr/local/lib/libprofiler.a" "LINKER:-lunwind")
# # Don't believe necessary, more for consistency
# add_library(profiler::profiler ALIAS profiler)


string(JOIN " " COMPILER_FLAGS "-Wall -Wextra -pedantic -Wfloat-equal"
"-Wwrite-strings -Wswitch-enum -Wunreachable-code -Wconversion"
"-Wcast-qual -Wstrict-overflow=5 -Werror=uninitialized"
"-Werror=return-type -pipe -fdiagnostics-color")

# string(APPEND CMAKE_CXX_FLAGS " ${COMPILER_FLAGS} ")
string(APPEND CMAKE_CXX_FLAGS " ${COMPILER_FLAGS} ")

# Don't want to build with sanitizers when installing with cmake or packaging,
# otherwise downstream users MUST also enable sanitizers

set(DEVELOPER_FLAGS FALSE CACHE BOOL "Enable warning/sanitizer flags during development of library")
if (DEVELOPER_FLAGS)
    set(SANITISERS " -fsanitize=address,undefined ")
endif()

string(APPEND CMAKE_CXX_FLAGS_DEBUG " -g -O0 -fno-omit-frame-pointer ${SANITISERS} ")
string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " -g -O2 -march=native -flto -fno-omit-frame-pointer ${SANITISERS} ")
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -g -O3 -march=native -flto -DNDEBUG ")

# string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG " -flto -fno-omit-frame-pointer ${SANITISERS} -fuse-ld=lld ")
# string(APPEND CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO " -flto -fno-omit-frame-pointer ${SANITISERS} -fuse-ld=lld ")
# string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " -flto -fuse-ld=lld ")
string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG " -flto -fno-omit-frame-pointer ")
string(APPEND CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO " -flto -fno-omit-frame-pointer ${SANITISERS} ")
string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " -flto ")


# # https://stackoverflow.com/a/10055571/8594193
# if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # # using Clang
    # string(APPEND CMAKE_CXX_FLAGS_RELEASE " -flto")
    # string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " -flto -fuse-ld=lld")
# elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # # using GCC
    # # Currently clangd 10 (but not 9) complains about "-fconcepts" so not using it.
    # # https://github.com/clangd/clangd/issues/331
    # # string(APPEND COMPILER_FLAGS " -fconcepts")
    # string(APPEND CMAKE_CXX_FLAGS_RELEASE " -flto")
    # string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " -flto -fuse-ld=lld")
# endif()

# set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 ${COMPILER_FLAGS}")
# set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O2 -fno-omit-frame-pointer -march=native ${COMPILER_FLAGS}")
# set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -g -O3 -march=native ${COMPILER_FLAGS}")

# set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${SANITISERS}")
# set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${SANITISERS}")


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

list(APPEND ALL_DICTIONARIES_CMAKE_LINK_LIBRARIES)
list(APPEND ALL_DICTIONARY_CLASSES)
list(APPEND ALL_SOLVER_INCLUDES)

macro(add_component snake_case_name _pascal_case_name)
    list(APPEND ALL_DICTIONARIES_CMAKE_LINK_LIBRARIES "${snake_case_name}::${snake_case_name}")
    message("Adding ${_pascal_case_name}")
    list(APPEND ALL_DICTIONARY_CLASSES "${snake_case_name}::${_pascal_case_name}")
    add_subdirectory(${snake_case_name})
    list(APPEND ALL_SOLVER_INCLUDES
        "#include \"${PROJECT_NAME}/${snake_case_name}/${snake_case_name}.hpp\"")
endmacro()

add_subdirectory(utility)

if ("" STREQUAL "${WORDSEARCH_SOLVERS}")
    message(FATAL_ERROR "Solvers specified in WORDSEARCH_SOLVERS may not be empty, must be semicolon separated list of items from ${ALL_SOLVERS}")
endif()

list(APPEND SOLVER_C_DEFINES)
string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPERCASE)
foreach(SOLVER IN LISTS WORDSEARCH_SOLVERS)
    if (NOT (SOLVER IN_LIST ALL_SOLVERS))
        message(FATAL_ERROR "Unrecognised solver \"${SOLVER}\", should be one of ${ALL_SOLVERS}")
    else()
        message("Found solver ${SOLVER}")
        list(FIND ALL_SOLVERS "${SOLVER}" index)
        message("Adding index ${index}")
        list(GET ALL_SOLVERS_CLASSNAMES ${index} pascal_case_name)
        message("Adding got solver classname ${pascal_case_name}")
        add_component("${SOLVER}" "${pascal_case_name}")
        list(APPEND SOLVER_C_DEFINES "#define ${PROJECT_NAME_UPPERCASE}_HAS_${SOLVER}")
    endif()
endforeach()
list(JOIN ALL_DICTIONARY_CLASSES ", " ALL_DICTIONARY_CLASSES)
list(JOIN ALL_SOLVER_INCLUDES "\n" ALL_SOLVER_INCLUDES)
list(JOIN SOLVER_C_DEFINES "\n" SOLVER_C_DEFINES)

string(JOIN "\n" config_file_content
    "#ifndef WORDSEARCH_CONFIG_HPP_IN"
    "#define WORDSEARCH_CONFIG_HPP_IN"
    "#define WORDSEARCH_DICTIONARY_CLASSES ${ALL_DICTIONARY_CLASSES}"
    "#include \"${PROJECT_NAME}/utility/utility.hpp\""
    "${ALL_SOLVER_INCLUDES}"
    "${SOLVER_C_DEFINES}"
    "#include \"${PROJECT_NAME}/solver/solver.hpp\""
    "#endif // WORDSEARCH_CONFIG_HPP_IN"
    )

set(PROJECT_GENERATED_HEADER_BUILD_PATH "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/${PROJECT_NAME}.hpp")
file(CONFIGURE
    OUTPUT "${PROJECT_GENERATED_HEADER_BUILD_PATH}"
    CONTENT "${config_file_content}")

configure_file(test/test_cases/massive_wordsearch.txt "${CMAKE_CURRENT_BINARY_DIR}" COPYONLY)

add_subdirectory(solver)

add_subdirectory(benchmark)

add_subdirectory(cmdline_app)

# add_subdirectory(compact_trie)
# add_subdirectory(compact_trie2)
# add_subdirectory(trie)
# add_subdirectory(dictionary_std_vector)
# add_subdirectory(dictionary_std_set)

# add_subdirectory(desktop_solver_gui)

# To add a new data structure for the solver, add it to:
# ALL_DICTIONARIES_CMAKE_LINK_LIBRARIES
# ALL_DICTIONARY_CLASSES
# and add the includes in test/test_cases.cpp and test/test_cases2.cpp
# Potential TODO: generate config header file here
# LIST(APPEND ALL_DICTIONARIES_CMAKE_LINK_LIBRARIES
    # "compact_trie::compact_trie"
    # "trie::trie"
    # "compact_trie2::compact_trie2"
    # "dictionary_std_vector::dictionary_std_vector"
    # "dictionary_std_set::dictionary_std_set"
    # )
# # These are used as template arguments for testing each dictionary
# string(JOIN ", " ALL_DICTIONARY_CLASSES
    # "compact_trie::CompactTrie"
    # "trie::Trie"
    # "compact_trie2::CompactTrie2"
    # "dictionary_std_vector::DictionaryStdVector"
    # "dictionary_std_set::DictionaryStdSet"
    # )

# add_library(config INTERFACE)
# add_library(config::config ALIAS config)
# target_sources(config INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/config.hpp")
# target_include_directories(config INTERFACE "${CMAKE_CURRENT_BINARY_DIR}")
# # Output dir is relative to CMAKE_CURRENT_BINARY_DIR by default
# configure_file(config.hpp.in config.hpp @ONLY)

# return()

include(GNUInstallDirs)

# Master/public library
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
target_sources(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/${PROJECT_NAME}.hpp>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${PROJECT_NAME}.hpp>
    )
target_link_libraries(${PROJECT_NAME} INTERFACE
    ${ALL_DICTIONARIES_CMAKE_LINK_LIBRARIES}
    solver::solver
    utility::utility
    )


# https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html
include(CMakePackageConfigHelpers)



# Installs the headers and libs
# install(TARGETS compact_trie)
# install(TARGETS trie)
# install(TARGETS compact_trie2)
# install(TARGETS dictionary_std_vector)
# install(TARGETS dictionary_std_set)
# install(TARGETS ws)
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets)

# set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADERS}")

# install(TARGETS ${PROJECT_NAME} PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/${PROJECT_NAME}")

# Generate a targets file and install it
install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
         )

####################################
# https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html

# Generates a package version file in the build directory
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    VERSION 0.1.5
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


install(FILES "${PROJECT_GENERATED_HEADER_BUILD_PATH}"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")

# Disabled for now because if we run in webpage we run out of memory
# If we run in node we get
# /home/justin/cpp/ws3/build/bin/the_test.js:40
# throw 'using preloaded data can only be done on a web page or in a web worker';
enable_testing()
add_subdirectory(test)

# add_library(ws INTERFACE)
# target_link_libraries(ws INTERFACE
    # "${ALL_DICTIONARIES_CMAKE_LINK_LIBRARIES}"
    # solver::solver
    # utility::utility
    # )


# include(CMakePackageConfigHelpers)
# include(GNUInstallDirs)

# set(PACKAGE_VERSION 0.1)
# write_basic_package_version_file(
    # wordsearch_solver-config-version.cmake
    # VERSION ${PACKAGE_VERSION}
    # COMPATIBILITY ExactVersion
    # )

# # install(EXPORT wordsearch_solver-targets.cmake
        # # DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/wordsearch_solver
        # # FILE wordsearch_solver-targets.cmake
        # # NAMESPACE wordsearch_solver::
         # # )

# install(
    # FILES wordsearch_solver-config.cmake
        # ${CMAKE_BINARY_DIR}/wordsearch_solver-config-version.cmake
    # DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/wordsearch_solver)

# set_target_properties(solver PROPERTIES PUBLIC_HEADER
    # "solver/include/solver.hpp;solver/include/solver.tpp")
# install(TARGETS utility)
# install(TARGETS solver)

# install(TARGETS compact_trie)
# install(TARGETS trie)
# install(TARGETS compact_trie2)
# install(TARGETS dictionary_std_vector)
# install(TARGETS dictionary_std_set)
# install(TARGETS ws)

# add_executable(t test.cpp)
# target_link_libraries(t ws)

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
    # message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

# install(FILES ws EXPORT wsconfig.cmake)
# Install the library and headers.
# install(TARGETS
    # solver utility
    # compact_trie
    # trie
    # compact_trie2
    # dictionary_std_vector
    # dictionary_std_set
    # EXPORT wsconfig.cmake
    # LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    # ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    # RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    # PRIVATE_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    # PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    # )
# install(FILES wsconfig.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/wordsearch_solver)
