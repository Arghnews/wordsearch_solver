#!/usr/bin/env bash

set -u -o pipefail
set -e

build_type=Debug
# wordsearch_solvers="trie;compact_trie;dictionary_std_vector"
wordsearch_solvers=""

if [ -n "$wordsearch_solvers" ]; then
    wordsearch_solvers="-DWORDSEARCH_SOLVERS=$wordsearch_solvers"
fi

(
rm -rf build && mkdir build
rm -rf z && mkdir z
cd build
export CMAKE_INSTALL_PREFIX=../z
    conan install -pr clang -s build_type=$build_type .. && \
        cmake -GNinja "$wordsearch_solvers" -DCMAKE_CXX_COMPILER=/usr/local/bin/clang++ -DCMAKE_BUILD_TYPE=$build_type -DCMAKE_INSTALL_PREFIX=../z .. && \
    cmake --build . -t all && cmake --build . -t install && tree ../z
)

