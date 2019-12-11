#!/usr/bin/env bash

set -u -o pipefail

# https://unix.stackexchange.com/a/326585/358344
# in /etc/default/grub: GRUB_CMDLINE_LINUX_DEFAULT+=" isolcpus=0"
# taskset 01 corresponds to cpu 0 that is isolated now

# CPUPROFILE=profile.prof taskset 01 build/wordsearch_solver_main -w build/test/test_cases/long_words/wordsearch.txt -d build/test/test_cases/dictionary.txt -b trie -q
# With editted version of gprof no need to set CPUPROFILE as it's hard coded in
# Getting again to place where optimised version is too fast for reliable
# sampling. Perhaps time to make a new bigger test case?
grep CMAKE_BUILD_TYPE build/CMakeCache.txt
if [ $# -eq 0 ]
then
    set -x
    CPUPROFILE_FREQUENCY=10000 taskset 01 build/wordsearch_solver_main -w build/test/test_cases/long_words/wordsearch.txt -d build/test/test_cases/dictionary.txt -b trie -q
    exit 0
fi

set -x
for arg in "$@"
do
    CPUPROFILE_FREQUENCY=10000 taskset 01 build/wordsearch_solver_main -w build/test/test_cases/long_words/wordsearch.txt -d build/test/test_cases/dictionary.txt -b "$arg" -q
done
