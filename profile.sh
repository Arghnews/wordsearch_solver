#!/usr/bin/env bash

set -o pipefail

# https://unix.stackexchange.com/a/326585/358344
# in /etc/default/grub: GRUB_CMDLINE_LINUX_DEFAULT+=" isolcpus=0"
# taskset 01 corresponds to cpu 0 that is isolated now

# Meant to be run from project root

function finish() {
    sudo cpupower frequency-set --governor powersave 1>/dev/null
}

trap finish EXIT

dict=test/test_cases/dictionary.txt
wordsearch=test/test_cases/massive_wordsearch.txt
solvers=("trie" "compact_trie" "compact_trie2" "dictionary_std_vector" "dictionary_std_set")

sudo cpupower frequency-set --governor performance 1>/dev/null # benchmark CPU scaling is enabled fix
mkdir -p profiles

for solver in "${solvers[@]}"
do
    (
    export CPUPROFILE=profiles/${solver}_profile.prof
    export CPUPROFILE_FREQUENCY=10000

    echo "$solver"
    taskset 01 build/cmdline_app/cmdline_app --dict $dict --wordsearch $wordsearch --solver $solver
    sudo cpupower frequency-set --governor powersave 1>/dev/null
    )
done

