#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CMAKE_DIR="$SCRIPT_DIR/../build/release"

sudo cmake --build "$CMAKE_DIR" --target make_distribution_tarball
