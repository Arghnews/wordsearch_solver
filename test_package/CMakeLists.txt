cmake_minimum_required(VERSION 3.18)
project(PackageTest CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_executable(example example.cpp)
message("conan libs: ${CONAN_LIBS}")
# target_link_libraries(example ${CONAN_LIBS})
target_link_libraries(example ${CONAN_LIBS})
target_compile_options(example PUBLIC "-fsanitize=address,undefined")
target_link_options(example PUBLIC "-fsanitize=address,undefined")

set_target_properties(example PROPERTIES
    CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
    # message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()


# CTest is a testing tool that can be used to test your project.
# enable_testing()
# add_test(NAME example
#          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
#          COMMAND example)
