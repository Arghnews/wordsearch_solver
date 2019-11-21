project(the_test)

find_package(fmt REQUIRED)
find_package(Catch2 REQUIRED)

# add_library(word_trie_lib SHARED word_trie_lib.h)

add_executable(the_test test_cases.cpp test_catch_main.cpp)
# add_executable(the_test test_catch_main.cpp)
target_link_libraries(the_test
  wordsearch_solver::wordsearch_solver
  Catch2::Catch2)

message("CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}")
message("CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")
message("CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}")
message("PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")
# message("
# 	copy ${PROJECT_SOURCE_DIR}/dictionary_gb.txt ${CMAKE_BINARY_DIR}")

# TODO: possibly add a config file thing to pass through test file names to cpp program via #defines ?
# If use simple folder copy/glob copy then don't track files?
add_custom_command(TARGET the_test POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E
	copy_directory
		"${PROJECT_SOURCE_DIR}/test_cases"
		"${CMAKE_CURRENT_BINARY_DIR}/test_cases"
	)
	# copy "${PROJECT_SOURCE_DIR}/dictionary_gb.txt" "${CMAKE_BINARY_DIR}"
	# copy "${PROJECT_SOURCE_DIR}/test_input.bigger" "${CMAKE_BINARY_DIR}"
# message("CMAKE_PROJECT_DIR ${CMAKE_PROJECT_DIR}")
# include(CTtest)

# add_test(test_all the_test)
include(CTest)
include(Catch)
catch_discover_tests(the_test)

#enable_testing()
##include
#catch_discover_tests(the_test)

# include(CTest)
# include(Catch)
# catch_discover_tests(the_test)
# target_compile_features(the_test PRIVATE cxx_std_17)
# set_target_properties(the_test PROPERTIES CXX_EXTENSIONS OFF)

# add_library(word_trie_lib SHARED IMPORTED GLOBAL)
# add_library(word_trie_lib::bla ALIAS word_trie_lib)
# add_library(word_trie_lib

# target_include_directories(the_test PRIVATE /home/justin/cpp/word_trie/src)
# add_subdirectory(src)
# add_subdirectory(test)
# enable_testing()
# catch_discover_tests(test)

