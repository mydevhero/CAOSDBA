#!/bin/bash
set -e

HIREDIS_SOURCE_DIR="$(cd $(dirname $0)/../hiredis && pwd)"
BUILD_DIR="$(pwd)/hiredis-build"
INSTALL_DIR="$(pwd)/hiredis-install"
PREBUILT_DIR="$(cd $(dirname $0)/../prebuilt && pwd)"

echo "Building hiredis static library..."

mkdir -p $BUILD_DIR $INSTALL_DIR
cd $HIREDIS_SOURCE_DIR

# Build hiredis
make ENABLE_STATIC=1 USE_SSL=1

# Install
make install PREFIX=$INSTALL_DIR ENABLE_STATIC=1

echo "hiredis built successfully in $INSTALL_DIR"

if [ -d "$PREBUILT_DIR" ]; then
    echo "Copying to vendor/prebuilt..."
    PREBUILT_DIR="$PREBUILT_DIR/hiredis"
    mkdir -p $PREBUILT_DIR/lib $PREBUILT_DIR/include
    cp -r $INSTALL_DIR/lib/* $PREBUILT_DIR/lib/
    cp -r $INSTALL_DIR/include/* $PREBUILT_DIR/include/
    echo "Copied to vendor/prebuilt"
else
    echo "vendor/prebuilt directory not found, skipping copy"
fi

echo "Libraries:"
ls -la $INSTALL_DIR/lib/
