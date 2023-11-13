#!/bin/sh -e

BASEDIR=$(realpath "$(dirname "$0")")
ROOTDIR=$(realpath "$BASEDIR/..")
MAIN=$(realpath "$ROOTDIR/build/main")
TESTDIR=$(realpath "$ROOTDIR/test")

printf "\033[0;32m%s\033[0m\n" "Normal input"
"$MAIN" < "$TESTDIR/normal_input.txt"

printf "\033[0;32m%s\033[0m\n" "Hacker input"
"$MAIN" < "$TESTDIR/hacker_input.txt" 2>&1 >/dev/null | grep other
