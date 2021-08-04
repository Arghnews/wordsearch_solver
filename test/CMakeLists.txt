project(the_test)

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

# set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(range-v3 REQUIRED)
find_package(fmt REQUIRED)
# find_package(solver_utility REQUIRED)
find_package(Catch2 REQUIRED)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

add_executable(${PROJECT_NAME} test_cases.cpp test_cases2.cpp test_catch_main.cpp test_cases.hpp)
# add_test(NAME ${PROJECT_NAME} COMMAND /usr/bin/node ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(${PROJECT_NAME} PUBLIC
    # utility::utility
    # range-v3::range-v3
    # fmt::fmt
    Catch2::Catch2
    fmt::fmt
    range-v3::range-v3
    wordsearch_solver::wordsearch_solver
    # "-O0"
    # solver_utility::solver_utility
    # config::config
    # solver::solver
    # Add -lstdc++fs if using clang and on linux
    $<IF:$<AND:$<CXX_COMPILER_ID:Clang>,$<PLATFORM_ID:Linux>>,"stdc++fs","">
  )

if(EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s DISABLE_EXCEPTION_CATCHING=2 --preload-file test_cases -O0 -g")
    file(COPY test_cases DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
endif()

# target_compile_options(${PROJECT_NAME} PUBLIC
    # $<$<BOOL:${EMSCRIPTEN}>:>)
# target_link_options(${PROJECT_NAME} PUBLIC
    # $<$<BOOL:${EMSCRIPTEN}>:--preload-file test_cases  >)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

# add_custom_target(copy_test_input_files ALL)
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${PROJECT_SOURCE_DIR}/test_cases"
		"${CMAKE_CURRENT_BINARY_DIR}/test_cases"
	)


if(NOT EMSCRIPTEN)
    add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
else()
    message(VERBOSE "CMAKE_CROSSCOMPILING_EMULATOR: ${CMAKE_CROSSCOMPILING_EMULATOR}")

    # Find path to node executable
    # It's defined in CMAKE_CROSSCOMPILING_EMULATOR but enclosed in quotes so
    # remove the quotes and store the result in node_path
    if (DEFINED CMAKE_CROSSCOMPILING_EMULATOR)
        string(REGEX REPLACE "^\"(.*)\"$" "\\1" node_path "${CMAKE_CROSSCOMPILING_EMULATOR}")
        message("node_path: ${node_path}")
    else()
        string(CONCAT msg "Expected to find path to node executable in variable "
            "CMAKE_CROSSCOMPILING_EMULATOR but variable not defined")
        message(FATAL_ERROR "${msg}")
    endif()

    set(test_executable "${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}")

    # We need to find the runtime output directory where the test executable will
    # be created. This may change depending on the build type.
    # If the build type is one of the usual 4, then use the property RUNTIME_OUTPUT_DIRECTORY_<CONFIG>
    # If the build type is not set, use the property of the test executable RUNTIME_OUTPUT_DIRECTORY
    # Else it's the empty string (probably should error but don't know how to error
    # in generator expressions)
    #
    # I don't think it's possible to make work something like
    # ${CMAKE_RUNTIME_OUTPUT_DIRECTORY$<CONFIG>}
    #
    # https://cmake.org/cmake/help/latest/command/add_test.html
    # https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html
    # https://cmake.org/cmake/help/latest/variable/CMAKE_RUNTIME_OUTPUT_DIRECTORY.html
    # https://cmake.org/cmake/help/latest/prop_tgt/RUNTIME_OUTPUT_DIRECTORY_CONFIG.html
    # https://cmake.org/cmake/help/latest/prop_tgt/RUNTIME_OUTPUT_DIRECTORY.html
    # https://cmake.org/cmake/help/latest/prop_tgt/RUNTIME_OUTPUT_DIRECTORY_CONFIG.html
    set(prop "\$\<TARGET_PROPERTY:${PROJECT_NAME}")
    string(CONCAT runtime_output_dir
        "$<IF:$<NOT:$<BOOL:$<CONFIG>>>,${prop},RUNTIME_OUTPUT_DIRECTORY>,"
            "$<IF:$<CONFIG:RELWITHDEBINFO>,${prop},RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO>,"
                "$<IF:$<CONFIG:MINSIZEREL>,${prop},RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL>,"
                    "$<IF:$<CONFIG:RELEASE>,${prop},RUNTIME_OUTPUT_DIRECTORY_RELEASE>,"
                        "$<IF:$<CONFIG:DEBUG>,${prop},RUNTIME_OUTPUT_DIRECTORY_DEBUG>,"
                            ""
                        ">"
                    ">"
                ">"
            ">"
        ">"
        )

    add_test(NAME ${PROJECT_NAME}
        COMMAND "${node_path}" "${test_executable}"
        WORKING_DIRECTORY "${runtime_output_dir}")
endif()

# message("Looking for camke stuff ${CMAKE_CURRENT_SOURCE_DIR}")
# message("Looking for camke stuff ${CMAKE_CURRENT_BINARY_DIR}")
# message("Looking for camke stuff ${CMAKE_CURRENT_LIST_DIR}")

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
    # message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

# include(CTest)
# include(Catch)
# catch_discover_tests(${PROJECT_NAME})

