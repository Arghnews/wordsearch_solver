#!/usr/bin/env bash

set -eu -o pipefail

# Quick script to build/upload a version

# TODO: add checks that we are incing version number
# Check that git is clean

build_types="Debug RelWithDebInfo Release"
# build_types="RelWithDebInfo Release"
# build_types="Release"

args=("-pr clang" "")
project="$(sed -E -n "s/^[[:space:]]*name = \"(.*)\"/\1/p" conanfile.py)"
version="$(sed -E -n "s/^[[:space:]]*version = \"(.*)\"/\1/p" conanfile.py)"

reference="$project/$version@arghnews/testing"
remote="my_jfrog"

: ${project:?"Must define project"}
: ${version:?"Must define version"}

echo "$reference"

for b in $build_types
do
    for a in "${args[@]}"; do
        set -x
        conan create $a -s build_type=$b . "$reference"
        # conan create -s build_type=$b . "$reference"
        set +x
    done
done

echo "#conan upload --all -r=$remote $reference"
