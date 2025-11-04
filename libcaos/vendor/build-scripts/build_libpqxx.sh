#!/bin/bash
set -e

LIBPQXX_SOURCE_DIR="$(cd $(dirname $0)/../libpqxx && pwd)"
POSTGRES_INSTALL_DIR="$1"
BUILD_DIR="$(pwd)/libpqxx-build"
INSTALL_DIR="$(pwd)/libpqxx-install"
PREBUILT_DIR="$(cd $(dirname $0)/../prebuilt && pwd)"

if [ -z "$POSTGRES_INSTALL_DIR" ] || [ ! -d "$POSTGRES_INSTALL_DIR" ]; then
    echo "PostgreSQL install directory not provided or invalid: $POSTGRES_INSTALL_DIR"
    echo "Usage: $0 <postgres_install_dir>"
    exit 1
fi

echo "Building libpqxx static library..."
echo "• Source: $LIBPQXX_SOURCE_DIR"
echo "• PostgreSQL: $POSTGRES_INSTALL_DIR"
echo "• Build: $BUILD_DIR"
echo "• Install: $INSTALL_DIR"

mkdir -p $BUILD_DIR $INSTALL_DIR
cd $BUILD_DIR

# Configure con CMake
cmake $LIBPQXX_SOURCE_DIR \
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
    -DPostgreSQL_TYPE_INCLUDE_DIR="$POSTGRES_INSTALL_DIR/include" \
    -DPostgreSQL_INCLUDE_DIR="$POSTGRES_INSTALL_DIR/include" \
    -DPostgreSQL_LIBRARY="$POSTGRES_INSTALL_DIR/lib/libpq.a" \
    -DPostgreSQL_LIBRARIES="$POSTGRES_INSTALL_DIR/lib/libpq.a;$POSTGRES_INSTALL_DIR/lib/libpgcommon.a;$POSTGRES_INSTALL_DIR/lib/libpgport.a"

# Build e install
cmake --build . --target pqxx
cmake --build . --target install

echo "libpqxx built successfully in $INSTALL_DIR"

if [ -d "$PREBUILT_DIR" ]; then
    echo "Copying to vendor/prebuilt..."

    LIBPQXX_PREBUILT="$PREBUILT_DIR/libpqxx"
    mkdir -p $LIBPQXX_PREBUILT/lib $LIBPQXX_PREBUILT/include

    cp -r $INSTALL_DIR/lib/* $LIBPQXX_PREBUILT/lib/
    cp -r $INSTALL_DIR/include/* $LIBPQXX_PREBUILT/include/

    echo "Copied to vendor/prebuilt/libpqxx"
else
    echo "vendor/prebuilt directory not found, skipping copy"
fi

echo "Libraries:"
ls -la $INSTALL_DIR/lib/
