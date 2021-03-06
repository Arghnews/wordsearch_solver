#!/usr/bin/env bash

set -u -o pipefail

# Go through all source files and convert includes of the form
# #include "trie/node.hpp" -> #include "wordsearch_solver/trie/node.hpp"

# for f in $(find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.tpp" -o -name "CMakeLists.txt" -o -name "*.py" -o -name "*.sh" \))
for f in $(find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.tpp" \))
do
    if [[ "$f" == "$0" ]]; then
        continue
    fi
    if [[ "$f" == *build/* ]]; then
        continue
    fi

    echo "$f"

    # sed -E -i "s/@copydoc trie::Trie::([^(]*)\(\)/@copydoc solver::SolverDictWrapper::\1()/" "$f"

    # Clang format everything
    # set -x
    # clang-format -i --style=file "$f"
    # set +x

    # Change cmake minimum version
    # # sed -i -E "s/^cmake_minimum_required\(VERSION (.*)\)$/cmake_minimum_required(VERSION 3.19)/" "$f"

    # line="$(grep "^#include \".*\"$" "$f")"
    # if [[ -z "$line" ]]
    # then
        # continue
    # fi

    # To see what will happen

    # sed -E -n "s/^#include \"(.*trie.*|utility\/.*|dictionary_.*|solver.*)\"$/#include \"wordsearch_solver\/\1\"/p" "$f"

    # grep -Hn --color=auto -i matrix "$f"
    # sed -E -i "s/SimpleMatrix/Matrix2d/g" "$f"

    # To make the changes
    # BE CAREFUL WITH THIS
    # sed -E -i "s/^#include \"(.*trie.*|utility\/.*|dictionary_.*|solver.*)\"$/#include \"wordsearch_solver\/\1\"/" "$f"

    # grep -Hn -E "cols_size|rows_size" "$f"
    # sed -n "s/cols_size/.columns()/p" "$f"

    :
done

