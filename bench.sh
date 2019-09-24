#!/usr/bin/env bash

set -u -o pipefail

name=""
overwrite=false
for arg in "$@"
do
    case "$arg" in
        -f)
            overwrite=true
            ;;
        *)
            name="$arg"
            ;;
    esac
done
: ${name:?Must set name}
name="${name}.png"
echo "Using name $name"
if [ -f "$name" ] && [ "$overwrite" = false ]
then
    echo "File exists, choose different name or add -f to force"
    exit 1
fi

program="build/word_trie"
program_args="examples/asdf.bigger"

time "$program" $program_args && \
perf record -g -- "$program" $program_args && \
    perf script | c++filt | \
    sed "s/std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >/string/g" | \
    sed "s/std:://g" | sed "s/__gnu_cxx:://g" | \
    sed "s/boost::container//g" | \
    gprof2dot \
    -f perf --skew 0.01 -n 1 -e 1 | \
    dot -Tpng -o "${name}" && \
    (nohup okular "${name}" &>/dev/null &)

# --node-label="total-time-percentage" \
# --node-label="self-time-percentage" \
# --node-label="total-time" \
