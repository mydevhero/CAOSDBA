#!/bin/bash
set -e

REDISPP_SOURCE_DIR="$(cd $(dirname $0)/../redis-plus-plus && pwd)"
HIREDIS_ARG="$1"
BUILD_DIR="$(pwd)/redispp-build"
INSTALL_DIR="$(pwd)/redispp-install"
PREBUILT_DIR="$(cd $(dirname $0)/../prebuilt && pwd)"

echo "Building redis-plus-plus static library..."
echo "• Source: $REDISPP_SOURCE_DIR"
echo "• hiredis: $HIREDIS_ARG"
echo "• Build: $BUILD_DIR"
echo "• Install: $INSTALL_DIR"

mkdir -p $BUILD_DIR $INSTALL_DIR
cd $BUILD_DIR

# Configura CMake in base al tipo di hiredis
if [ "$HIREDIS_ARG" = "system" ]; then
    echo "Using system hiredis"
    cmake $REDISPP_SOURCE_DIR \
        -DCMAKE_BUILD_TYPE=Release \
        -DREDIS_PLUS_PLUS_BUILD_STATIC=ON \
        -DREDIS_PLUS_PLUS_BUILD_SHARED=OFF \
        -DREDIS_PLUS_PLUS_USE_TLS=OFF \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"
else
    echo "Using custom hiredis: $HIREDIS_ARG"
    cmake $REDISPP_SOURCE_DIR \
        -DCMAKE_BUILD_TYPE=Release \
        -DREDIS_PLUS_PLUS_BUILD_STATIC=ON \
        -DREDIS_PLUS_PLUS_BUILD_SHARED=OFF \
        -DREDIS_PLUS_PLUS_USE_TLS=OFF \
        -DCMAKE_PREFIX_PATH="$HIREDIS_ARG" \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"
fi

# Build e install
cmake --build . --target install -- -j$(nproc)

echo "redis-plus-plus built successfully in $INSTALL_DIR"

if [ -d "$PREBUILT_DIR" ]; then
    echo "Copying to vendor/prebuilt..."
    PREBUILT_REDISPP_DIR="$PREBUILT_DIR/redispp"
    mkdir -p $PREBUILT_REDISPP_DIR/lib $PREBUILT_REDISPP_DIR/include
    cp -r $INSTALL_DIR/lib/* $PREBUILT_REDISPP_DIR/lib/
    cp -r $INSTALL_DIR/include/* $PREBUILT_REDISPP_DIR/include/
    echo "Copied to vendor/prebuilt/redispp"
else
    echo "vendor/prebuilt directory not found, skipping copy"
fi

echo "Libraries:"
ls -la $INSTALL_DIR/lib/
echo "Includes:"
ls -la $INSTALL_DIR/include/
