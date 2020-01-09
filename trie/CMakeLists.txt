cmake_minimum_required(VERSION 3.14)
project(trie)

find_package(jr_assert REQUIRED)
find_package(fmt REQUIRED)

add_library(${PROJECT_NAME} INTERFACE)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

include(GNUInstallDirs)

set(INSTALL_INCLUDE_DIRECTORY ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIRECTORY}>)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER
    include/trie.h)

target_link_libraries(${PROJECT_NAME}
    INTERFACE
    jr_assert::jr_assert
    fmt::fmt
    )

# install(TARGETS ${PROJECT_NAME}
    # EXPORT ${PROJECT_NAME}-targets
    # PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDE_DIRECTORY})
    # ARCHIVE DESTINATION lib
    # LIBRARY DESTINATION lib
    # RUNTIME DESTINATION bin)

# install(EXPORT ${PROJECT_NAME}-targets
    # NAMESPACE ${PROJECT_NAME}::
    # FILE ${PROJECT_NAME}-config.cmake
    # DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# install(DIRECTORY ${PROJECT_SOURCE_DIR}/include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# install(DIRECTORY ${PROJECT_SOURCE_DIR}/include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jr_assert)
# install(DIRECTORY ${PROJECT_SOURCE_DIR}/include
    # DESTINATION include)

# TODO: add CMake requires compiler features/version etc.
