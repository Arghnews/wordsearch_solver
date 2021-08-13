cmake_minimum_required(VERSION 3.19)

project(utility CXX)

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

find_package(range-v3 REQUIRED)
find_package(fmt REQUIRED)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(INSTALL_INCLUDE_DIR "include")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(HEADERS "utility.hpp" "utility.tpp" "flat_char_value_map.hpp")
set(SOURCES "utility.cpp")

list(TRANSFORM HEADERS PREPEND "${INCLUDE_DIR}/wordsearch_solver/${PROJECT_NAME}/")
list(TRANSFORM SOURCES PREPEND "${SRC_DIR}/")

add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
    )

target_link_libraries(${PROJECT_NAME} PUBLIC
    fmt::fmt
    range-v3::range-v3
    link_std_filesystem::link_std_filesystem
    )

include(GNUInstallDirs)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets
    PUBLIC_HEADER
    DESTINATION "${INSTALL_INCLUDE_DIR}/wordsearch_solver/${PROJECT_NAME}")

install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/wordsearch_solver/${PROJECT_NAME}"
         )


# target_link_libraries(${PROJECT_NAME} INTERFACE ${CONAN_LIBS} -lstdc++fs)
	# INTERFACE
    # fmt::fmt
    # range-v3::range-v3
    # -lstdc++fs
    # )
