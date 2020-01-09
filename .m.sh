#!/usr/bin/env bash

set -u -o pipefail

(
    declare -a cmake_args

    has_cmake_generator=false
    has_CXX_COMPILER=false

    for arg in "$@"
    do
        # echo "The arg is: $arg"
        case "$arg" in
            -G*)
                # echo "On the case of -G $arg"
                has_cmake_generator=true
                ;;
            -DCMAKE_CXX_COMPILER*)
                # echo "On the case of -G $arg"
                has_CXX_COMPILER=true
                ;;
            *)
                # echo "Else case $arg"
                ;;
        esac
        cmake_args+=("$arg")
    done

    if [ "$has_cmake_generator" = false ]
    then
        cmake_args+=("-GNinja")
    fi
    if [ "$has_CXX_COMPILER" = false ]
    then
        cmake_args+=("-DCMAKE_CXX_COMPILER=/usr/bin/g++")
    fi

    numb_cpus="$(grep -c processor /proc/cpuinfo)"

    # for arg in "${cmake_args[@]}"
    # do
        # echo "Arg is: $arg"
    # done
    # echo "${cmake_args[@]}"

    cd build && \
        cmake "${cmake_args[@]}" .. && \
        cmake --build . --parallel "$numb_cpus" && \
        CTEST_OUTPUT_ON_FAILURE=1 cmake --build . --target test && \
        cd .. && \
        after_build_insert_executable_on_line
    # cd build
    # d="$(pwd)"
    # ./wordsearch_solver_main -d "$d/test/test_cases/dictionary.txt" -w "$d/test/test_cases/tiny_test/wordsearch.txt"
    # ( cd build && make test || ./word_trie_main )
    :
)
