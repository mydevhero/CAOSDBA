#!/bin/bash
set -e

FMT_SOURCE_DIR="$(cd $(dirname $0)/../fmt && pwd)"
BUILD_DIR="$(pwd)/fmt-build"
INSTALL_DIR="$(pwd)/fmt-install"
PREBUILT_DIR="$(cd $(dirname $0)/../prebuilt && pwd)"

echo "Configuring fmt..."
echo "• Source: $FMT_SOURCE_DIR"
echo "• Build: $BUILD_DIR"
echo "• Install: $INSTALL_DIR"

mkdir -p $BUILD_DIR $INSTALL_DIR
cd $BUILD_DIR

# Configure fmt
cmake $FMT_SOURCE_DIR \
    -DCMAKE_BUILD_TYPE=Release \
    -DFMT_TEST=OFF \
    -DFMT_DOC=OFF \
    -DFMT_INSTALL=ON \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"

# Build (fmt ha componenti compilati per ottimizzazione)
cmake --build .

# Install
cmake --build . --target install

echo "fmt configured successfully in $INSTALL_DIR"

if [ -d "$PREBUILT_DIR" ]; then
    echo "Copying to vendor/prebuilt..."

    FMT_PREBUILT="$PREBUILT_DIR/fmt"
    mkdir -p $FMT_PREBUILT/include $FMT_PREBUILT/lib

    # Copia headers
    if [ -d "$INSTALL_DIR/include" ]; then
        cp -r $INSTALL_DIR/include/* $FMT_PREBUILT/include/
    fi

    # Copia libraries (fmt ha libfmt.a per ottimizzazioni)
    if [ -d "$INSTALL_DIR/lib" ]; then
        cp -r $INSTALL_DIR/lib/* $FMT_PREBUILT/lib/ 2>/dev/null || true
    fi

    # Copia anche dalla source dir
    cp -r $FMT_SOURCE_DIR/include/* $FMT_PREBUILT/include/ 2>/dev/null || true

    echo "Copied to vendor/prebuilt/fmt"
else
    echo "vendor/prebuilt directory not found, skipping copy"
fi

echo "Files installed:"
find $INSTALL_DIR -type f 2>/dev/null || echo "No files in install directory"
