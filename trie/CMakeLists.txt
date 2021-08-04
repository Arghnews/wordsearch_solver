cmake_minimum_required(VERSION 3.18)

project(trie)

# set(CONAN_DISABLE_CHECK_COMPILER TRUE)
# set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

find_package(range-v3 REQUIRED)
find_package(fmt REQUIRED)
find_package(Boost REQUIRED QUIET COMPONENTS container)
# find_package(llvm_small_vector REQUIRED)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(INSTALL_INCLUDE_DIR "include")

set(HEADERS "trie.hpp" "trie.tpp" "node.hpp")
set(SOURCES "node.cpp" "trie.cpp")

list(TRANSFORM HEADERS PREPEND "${INCLUDE_DIR}/wordsearch_solver/${PROJECT_NAME}/")
list(TRANSFORM SOURCES PREPEND "${SRC_DIR}/")

add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
    )

target_link_libraries(${PROJECT_NAME} PUBLIC
    fmt::fmt
    range-v3::range-v3
    utility::utility
    Boost::container
    )

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets PUBLIC_HEADER
    DESTINATION "${INSTALL_INCLUDE_DIR}/wordsearch_solver/${PROJECT_NAME}")

include(GNUInstallDirs)

install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/wordsearch_solver/${PROJECT_NAME}"
         )

# make_compile_commands(
    # TARGET_TO_REBUILD_ON ${PROJECT_NAME}
    # DESTINATION "${PROJECT_SOURCE_DIR}")

# # EXCLUDE_FROM_ALL
# add_executable(${PROJECT_NAME}_main main.cpp)
# target_link_libraries(${PROJECT_NAME}_main ${PROJECT_NAME}::${PROJECT_NAME} "-O0")

# set_target_properties(${PROJECT_NAME}_main PROPERTIES
    # CXX_STANDARD 17
    # CXX_STANDARD_REQUIRED ON
    # CXX_EXTENSIONS OFF)

