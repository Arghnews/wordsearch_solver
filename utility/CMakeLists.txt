cmake_minimum_required(VERSION 3.19)

project(utility CXX)

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

find_package(range-v3 REQUIRED)
find_package(fmt REQUIRED)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(INSTALL_INCLUDE_DIR "include")
# set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(HEADERS "utility.hpp" "flat_char_value_map.hpp")

list(TRANSFORM HEADERS PREPEND "${INCLUDE_DIR}/wordsearch_solver/${PROJECT_NAME}/")
# list(TRANSFORM SOURCES PREPEND "${SOURCE_DIR}/")

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

# set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# I'm a retard this doesn't work in build, even if the generator crap lets it work for the installed...

target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
    )

# We need to pass -stdc++fs as a link flag here. We could do this in conan
# in package_info with self.cpp_info.libs += ["-lstdc++fs"]
# However that means that consumers of this from cmake won't get the flag
# so we set it here instead. This assumes libstdc++.

target_link_libraries(${PROJECT_NAME} INTERFACE
    fmt::fmt
    range-v3::range-v3
    $<IF:$<AND:$<CXX_COMPILER_ID:Clang>,$<PLATFORM_ID:Linux>>,"stdc++fs","">
    # -lstdc++fs
    )

include(GNUInstallDirs)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}-targets PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/wordsearch_solver/${PROJECT_NAME}")

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
