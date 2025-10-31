#!/bin/bash
set -e

REDISPP_SOURCE_DIR="$(cd $(dirname $0)/../redis-plus-plus && pwd)"
HIREDIS_INSTALL_DIR="$1"
BUILD_DIR="$(pwd)/redispp-build"
INSTALL_DIR="$(pwd)/redispp-install"
PREBUILT_DIR="$(cd $(dirname $0)/../prebuilt && pwd)"

if [ -z "$HIREDIS_INSTALL_DIR" ] || [ ! -d "$HIREDIS_INSTALL_DIR" ]; then
    echo "❌ hiredis install directory not provided or invalid: $HIREDIS_INSTALL_DIR"
    echo "Usage: $0 <hiredis_install_dir>"
    exit 1
fi

echo "🔨 Building redis-plus-plus static library..."
echo "• Source: $REDISPP_SOURCE_DIR"
echo "• hiredis: $HIREDIS_INSTALL_DIR"
echo "• Build: $BUILD_DIR"
echo "• Install: $INSTALL_DIR"

mkdir -p $BUILD_DIR $INSTALL_DIR
cd $BUILD_DIR

# Configure con CMake
cmake $REDISPP_SOURCE_DIR \
    -DCMAKE_BUILD_TYPE=Release \
    -DREDIS_PLUS_PLUS_BUILD_STATIC=ON \
    -DREDIS_PLUS_PLUS_BUILD_SHARED=OFF \
    -DCMAKE_PREFIX_PATH="$HIREDIS_INSTALL_DIR" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"

# Build e install
cmake --build . --target install

echo "✅ redis-plus-plus built successfully in $INSTALL_DIR"

if [ -d "$PREBUILT_DIR" ]; then
    echo "📦 Copying to vendor/prebuilt..."
    PREBUILT_DIR="$PREBUILT_DIR/redispp"
    mkdir -p $PREBUILT_DIR/lib $PREBUILT_DIR/include
    cp -r $INSTALL_DIR/lib/* $PREBUILT_DIR/lib/
    cp -r $INSTALL_DIR/include/* $PREBUILT_DIR/include/
    echo "✅ Copied to vendor/prebuilt"
else
    echo "⚠️  vendor/prebuilt directory not found, skipping copy"
fi

echo "📁 Libraries:"
ls -la $INSTALL_DIR/lib/
