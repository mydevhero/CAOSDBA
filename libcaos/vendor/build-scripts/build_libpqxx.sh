#!/bin/bash
set -e

LIBPQXX_SOURCE_DIR="$(cd $(dirname $0)/../libpqxx && pwd)"
POSTGRES_INSTALL_DIR="$1"
BUILD_DIR="$(pwd)/libpqxx-build"
INSTALL_DIR="$(pwd)/libpqxx-install"
PREBUILT_DIR="$(cd $(dirname $0)/../prebuilt && pwd)"

if [ -z "$POSTGRES_INSTALL_DIR" ] || [ ! -d "$POSTGRES_INSTALL_DIR" ]; then
    echo "Error: missing PostgreSQL install dir"
    exit 1
fi

echo "Building libpqxx static library..."
echo "• Source:     $LIBPQXX_SOURCE_DIR"
echo "• PostgreSQL: $POSTGRES_INSTALL_DIR"

mkdir -p "$BUILD_DIR" "$INSTALL_DIR"
cd "$BUILD_DIR"

cmake "$LIBPQXX_SOURCE_DIR" \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_C_FLAGS="-fPIC" \
    -DCMAKE_CXX_FLAGS="-fPIC" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DPQXX_BUILD_DOC=OFF \
    -DPQXX_BUILD_TESTS=OFF \
    -DSKIP_BUILD_TEST=ON \
    -DCMAKE_PREFIX_PATH="$POSTGRES_INSTALL_DIR" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DPostgreSQL_INCLUDE_DIR="$POSTGRES_INSTALL_DIR/include" \
    -DPostgreSQL_LIBRARY="$POSTGRES_INSTALL_DIR/lib/libpq.a"

cmake --build . --target pqxx
cmake --build . --target install

echo "libpqxx built successfully at $INSTALL_DIR"

if [ -d "$PREBUILT_DIR" ]; then
    mkdir -p "$PREBUILT_DIR/libpqxx/lib" "$PREBUILT_DIR/libpqxx/include"
    cp -r "$INSTALL_DIR/lib/"* "$PREBUILT_DIR/libpqxx/lib/"
    cp -r "$INSTALL_DIR/include/"* "$PREBUILT_DIR/libpqxx/include/"
fi

ls -la "$INSTALL_DIR/lib/"
