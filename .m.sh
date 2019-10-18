#!/usr/bin/env bash

set -u -o pipefail

(
    cd build/test && CTEST_OUTPUT_ON_FAILURE=1 cmake --build . --target test && cd ../.. && after_build_insert_executable_on_line
    # ( cd build && make test || ./word_trie_main )
)
