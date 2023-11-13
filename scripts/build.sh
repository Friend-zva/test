#!/bin/sh -e

BASEDIR=$(realpath "$(dirname "$0")")
ROOTDIR=$(realpath "$BASEDIR/..")
BUILDDIR=$(realpath "$ROOTDIR/build")

cmake -S . -B "$BUILDDIR"
cmake --build "$BUILDDIR"
