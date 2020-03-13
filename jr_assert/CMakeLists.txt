cmake_minimum_required(VERSION 3.14)

if (TARGET utility)
    return()
endif()

project(jr_assert)

if (NOT TARGET fmt)
	find_package(fmt REQUIRED)
endif()

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_VERBOSE_MAKEFILE ON)

add_library(${PROJECT_NAME} INTERFACE)
# target_sources(${PROJECT_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/jr_assert/jr_assert.h")
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

# message("Install interface: ${INSTALL_INTERFACE}")
# message("CMAKE_INSTALL_INCLUDEDIR: ${CMAKE_INSTALL_INCLUDEDIR}")

set(INSTALL_INTERFACE_DIRECTORY "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")

target_include_directories(${PROJECT_NAME}
    SYSTEM INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )

# Will shut up the gnu variadic args extension warning
target_include_directories(${PROJECT_NAME}
    SYSTEM INTERFACE
    $<INSTALL_INTERFACE:${INSTALL_INTERFACE_DIRECTORY}>
    )

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME}
        INTERFACE
        -Wno-gnu-zero-variadic-macro-arguments
        )
endif()

target_link_libraries(${PROJECT_NAME}
    INTERFACE
    fmt::fmt
    )

# set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-Wno-gnu-zero-variadic-macro-arguments")

# set(DESTDIR /home/justin/cpp/trie/${PROJECT_NAME}/tmp)
# set(CMAKE_INSTALL_PREFIX /home/justin/cpp/trie/${PROJECT_NAME}/tmp)

    # set_target_properties(mylib PROPERTIES PUBLIC_HEADER  include/mylib.h")
set_target_properties(${PROJECT_NAME} PROPERTIES
	PUBLIC_HEADER "include/jr_assert/jr_assert.h;include/jr_assert/hedley/hedley.h"
	)
# target_sources(${PROJECT_NAME} INTERFACE
    # $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/jr_assert.h>
    # $<INSTALL_INTERFACE:${INSTALL_INTERFACE_DIRECTORY}/jr_assert.h>)

# string(CONCAT COMPILER_FLAGS "-Wall -Wextra -pedantic -Wfloat-equal "
    # "-Wwrite-strings -Wswitch-enum -Wunreachable-code -Wconversion "
    # "-Wcast-qual -Wstrict-overflow=5 -Werror=uninitialized "
    # "-Werror=return-type -pipe -fdiagnostics-color")
# set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fno-omit-frame-pointer -march=native ${COMPILER_FLAGS}")
# set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-fsanitize=address,undefined")
# set(CMAKE_BUILD_TYPE "Debug")
# add_executable(main main.cpp)
# target_link_libraries(main PRIVATE jr_assert::jr_assert -g fmt::fmt)
# target_compile_definitions(main PRIVATE NDEBUG JR_ASSERT_DISABLE)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    PUBLIC_HEADER DESTINATION ${INSTALL_INTERFACE_DIRECTORY}
    )
    # ARCHIVE DESTINATION lib
    # LIBRARY DESTINATION lib
    # RUNTIME DESTINATION bin)

install(EXPORT ${PROJECT_NAME}-targets
    NAMESPACE ${PROJECT_NAME}::
    FILE ${PROJECT_NAME}-config.cmake
    DESTINATION lib/cmake/${PROJECT_NAME})

# install(DIRECTORY ${PROJECT_SOURCE_DIR}/include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# install(DIRECTORY ${PROJECT_SOURCE_DIR}/include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
# install(DIRECTORY ${PROJECT_SOURCE_DIR}/include
    # DESTINATION include)

# TODO: add CMake requires compiler features/version etc.
