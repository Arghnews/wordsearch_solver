#!/usr/bin/env bash

set -u -o pipefail
set -e

build_type=Debug
# build_type=RelWithDebInfo
# build_type=Release

wordsearch_solvers=""
# wordsearch_solvers="trie;compact_trie;dictionary_std_vector"
# wordsearch_solvers="trie"
# wordsearch_solvers="compact_trie"
# wordsearch_solvers="dictionary_std_vector;trie"

compiler=clang
# compiler=gcc

# build_cmdline_app="OFF"
build_cmdline_app="ON"

###############################################################################

if [ -n "$wordsearch_solvers" ]; then
    wordsearch_solvers="-DWORDSEARCH_SOLVERS=$wordsearch_solvers"
fi

sanitiser=""
if [[ "$build_type" == "Debug" ]]; then
    sanitiser="-DDEVELOPER_FLAGS=ON"
fi

profile=""
cxx_compiler=""
if [[ "$compiler" == "clang" ]]; then
    profile="-pr clang"
    cxx_compiler="-DCMAKE_CXX_COMPILER=/usr/local/bin/clang++"
fi

(
rm -rf build && mkdir build
rm -rf z && mkdir z
cd build
export CMAKE_INSTALL_PREFIX=../z

conan install $profile -s build_type=$build_type --build=missing .. && \
    cmake -GNinja "$sanitiser" "$wordsearch_solvers" "$cxx_compiler" \
    -DWORDSEARCH_SOLVER_BUILD_CMDLINE_APP=$build_cmdline_app \
    -DCMAKE_BUILD_TYPE=$build_type -DCMAKE_INSTALL_PREFIX=../z .. && \
# cmake --build . -t all && cmake --build . -t install && tree ../z

nice -n 19 cmake --build . && cmake --build . -t install && tree ../z \
    && cd .. && ./test.sh

)

