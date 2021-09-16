#!/usr/bin/env bash

# set -u -o pipefail
set -u
# Don't set -o pipefail for this, it doesn't play nicely with gnu parallel and
# its exit code. Notably, when a command run by gnu parallel completes
# successfully, it reports a non zero (71) exit code.

# Based on
# https://github.com/catchorg/Catch2/issues/399#issuecomment-672391192
# To parallelise catch2 tests

command_exists ()
{
    command -v "$1" &> /dev/null
}

(

cd build/test

cores=20
if command_exists nproc; then
    cores=$(nproc)
fi

# mkdir "tmp$$"
exe=./the_test

# $exe --list-test-names-only --order lex | parallel --will-cite --bar --halt now,fail=1 -j $cores ./the_test --use-colour=yes
$exe --list-test-names-only --order lex | parallel --will-cite --bar --halt now,fail=1 -j $cores ./the_test --use-colour=yes
ex=$?
if [ $ex -ne 0 ]; then
    IFS='' read -r -d '' warn <<'EOF'



    *********************************************************************
    WARNING: Some test cases failed - you may need to scroll up to find them
    *********************************************************************

EOF

    echo "$warn"
    echo "Exit status: $ex"
    exit $ex
fi
# $exe --list-test-names-only --order lex  |
    # parallel --will-cite --bar --halt now,fail=1 -j "$cores" $exe "{}" ">" "tmp$$/{}.out"
)
