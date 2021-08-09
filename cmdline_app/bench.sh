#!/usr/bin/env bash

set -o pipefail

# https://unix.stackexchange.com/a/326585/358344
# in /etc/default/grub: GRUB_CMDLINE_LINUX_DEFAULT+=" isolcpus=0"
# taskset 01 corresponds to cpu 0 that is isolated now


# Meant to be run from project root


dict=test/test_cases/dictionary.txt
wordsearch=test/test_cases/massive_wordsearch.txt
solvers=("trie" "compact_trie" "compact_trie2" "dictionary_std_vector" "dictionary_std_set")

for solver in "${solvers[@]}"
do
    (
    cd ..
    sudo cpupower frequency-set --governor performance 1>/dev/null # benchmark CPU scaling is enabled fix
    export CPUPROFILE=profile.prof
    export CPUPROFILE_FREQUENCY=10000

    echo "$solver"
    taskset 01 build/cmdline_app/cmdline_app --dict $dict --wordsearch $wordsearch --solver $solver
    sudo cpupower frequency-set --governor powersave 1>/dev/null
    )
done

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
