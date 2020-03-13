#!/usr/bin/env bash

set -u -o pipefail

 # ( cd build && cmake .. "$@" && cmake --build . --parallel "$numb_cpus" )
# set -x

(
sudo rm -rf tmp && mkdir tmp && cmake -S . -B build -DCMAKE_INSTALL_PREFIX=tmp \
    && cmake --build build && sudo cmake --build build --target install \
    && tree tmp
)
