#!/usr/bin/env bash

set -o pipefail

function finish() {
    sudo cpupower frequency-set --governor powersave 1>/dev/null
}

trap finish EXIT

(
# benchmark CPU scaling is enabled fix
sudo cpupower frequency-set --governor performance 1>/dev/null
cd build/benchmark

taskset 01 ./bench --benchmark_filter=init

taskset 01 ./bench --benchmark_filter=long_words
)

# https://unix.stackexchange.com/a/326585/358344
# in /etc/default/grub: GRUB_CMDLINE_LINUX_DEFAULT+=" isolcpus=0"
# taskset 01 corresponds to cpu 0 that is isolated now

# NOTE: This file is old. The comment above, and the command below, are what is
# useful to remember
# CPUPROFILE=profile.prof taskset 01 build/wordsearch_solver_main -w build/test/test_cases/long_words/wordsearch.txt -d build/test/test_cases/dictionary.txt -b trie -q

# With editted version of gprof no need to set CPUPROFILE as it's hard coded in
# Getting again to place where optimised version is too fast for reliable
# sampling. Perhaps time to make a new bigger test case?
# grep CMAKE_BUILD_TYPE build/CMakeCache.txt

# # sudo cpupower frequency-set --governor performance # benchmark CPU scaling is enabled fix

# # if [ $# -eq 0 ]
# # then

# binary=build/wordsearch_solver_main

# if [ -z "$1" ]
# then
    # echo "Must provide solver type"
    # $binary --help
    # exit 1
# fi

# for solver in "$@"
# do
    # set -x
    # CPUPROFILE=profile.prof CPUPROFILE_FREQUENCY=10000 \
        # taskset 01 build/wordsearch_solver_main -w build/test/test_cases/long_words/wordsearch.txt -d build/test/test_cases/dictionary.txt -b "$solver" -q
    # done
# exit 0
# # fi

# set -x
# # for arg in "$@"
# # do
    # CPUPROFILE_FREQUENCY=10000 taskset 01 build/wordsearch_solver_main -w build/test/test_cases/long_words/wordsearch.txt -d build/test/test_cases/dictionary.txt -b "$arg" -q
# done
