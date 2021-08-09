cmake_minimum_required(VERSION 3.19)

project(solver)

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})
# Assumed to be wordsearch_solver
set(PARENT_PROJECT "${CMAKE_PROJECT_NAME}")

find_package(fmt REQUIRED)
find_package(range-v3 REQUIRED)
# find_package(solver_utility REQUIRED)
find_package(Boost REQUIRED QUIET COMPONENTS container)
# find_package(static_vector REQUIRED)
find_package(matrix2d REQUIRED)

# Error:
# Cannot enlarge memory arrays to size 52154368 bytes (OOM). Either (1) compile with  -s INITIAL_MEMORY=X  with X higher than the current value 50331648, (2) compile with  -s ALLOW_MEMORY_GROWTH=1  which allows increasing the size at runtime, or (3) if you want malloc to return NULL (0) instead of this abort, compile with  -s ABORTING_MALLOC=0

# if (NOT EMSCRIPTEN)
    # find_package(google-profiler REQUIRED)
# endif()

# Project wide cpp standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# set(CMAKE_VERBOSE_MAKEFILE ON)

# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
# conan_basic_setup()

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(INSTALL_INCLUDE_DIR "include")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(HEADERS "solver.hpp" "solver.tpp")
set(SOURCES "solver.cpp")

list(TRANSFORM HEADERS PREPEND "${INCLUDE_DIR}/${PARENT_PROJECT}/${PROJECT_NAME}/")
list(TRANSFORM SOURCES PREPEND "${SRC_DIR}/")

add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
    )

# if (NOT EMSCRIPTEN)
    # set(gprof "google-profiler::google-profiler")
# endif()

target_link_libraries(${PROJECT_NAME} PUBLIC
    matrix2d::matrix2d
    fmt::fmt
    range-v3::range-v3
    utility::utility
    # "-s INITIAL_MEMORY=67108864"
    # "-s DISABLE_EXCEPTION_CATCHING=0"
    # "-O0"
    ${gprof}
    "${ALL_DICTIONARIES_CMAKE_LINK_LIBRARIES}"
    Boost::container
    config::config
    )

# EXCLUDE_FROM_ALL
# add_executable(${PROJECT_NAME}_main main.cpp)
# target_link_libraries(${PROJECT_NAME}_main ${PROJECT_NAME}::${PROJECT_NAME})

include(GNUInstallDirs)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets PUBLIC_HEADER
    DESTINATION "${INSTALL_INCLUDE_DIR}/${PARENT_PROJECT}/${PROJECT_NAME}")

install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PARENT_PROJECT}/${PROJECT_NAME}"
         )

# Removed temporarily to allow cmake build only this
 # make_compile_commands(
    # TARGET_TO_REBUILD_ON "${PROJECT_NAME}"
    # DESTINATION "${PROJECT_SOURCE_DIR}")

