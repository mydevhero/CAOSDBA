#!/bin/bash
set -e

MARIADB_SOURCE_DIR="$(cd $(dirname $0)/../mariadb-connector-cpp && pwd)"
BUILD_DIR="$(pwd)/mariadb-connector-build"
INSTALL_DIR="$(pwd)/mariadb-connector-install"
PREBUILT_DIR="$(cd $(dirname $0)/../prebuilt && pwd)"

echo "Building MariaDB connector static libraries (with -fPIC)..."
echo "• Source:  $MARIADB_SOURCE_DIR"
echo "• Build:   $BUILD_DIR"
echo "• Install: $INSTALL_DIR"

# Check if source directory exists
if [ ! -d "$MARIADB_SOURCE_DIR" ]; then
    echo "ERROR: MariaDB source directory not found: $MARIADB_SOURCE_DIR"
    echo "Did you initialize the submodule? Run: git submodule update --init --recursive"
    exit 1
fi

rm -rf "$BUILD_DIR" "$INSTALL_DIR"
mkdir -p "$BUILD_DIR" "$INSTALL_DIR"

cd "$BUILD_DIR"

# -----------------------------------
# CONFIGURE FOR STATIC BUILD WITH POSITION INDEPENDENT CODE
# -----------------------------------
echo "Configuring MariaDB connector for static build with -fPIC..."

if ! cmake "$MARIADB_SOURCE_DIR" \
  -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_CXX_FLAGS="-fPIC" \
  -DCMAKE_C_FLAGS="-fPIC" \
  -DCMAKE_BUILD_TYPE=Release \
  -DWITH_EXTERNAL_ZLIB=ON \
  -DWITH_MYSQLCOMPAT=ON \
  -DWITH_UNIT_TESTS=OFF \
  -DWITH_EXAMPLES=OFF \
  -DBUILD_SHARED_LIBS=OFF; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi

# -----------------------------------
# BUILD
# -----------------------------------
echo "Building MariaDB connector..."
if ! make -j$(nproc); then
    echo "ERROR: Build failed"
    exit 1
fi

# -----------------------------------
# INSTALL
# -----------------------------------
echo "Installing MariaDB connector..."
if ! make install; then
    echo "ERROR: Install failed"
    exit 1
fi

echo "MariaDB connector built successfully."

# -----------------------------------
# VERIFY THE BUILD RESULT
# -----------------------------------
echo "Checking installation directory structure..."
find "$INSTALL_DIR" -type f -name "*.a" | while read file; do
    echo "• Found library: $file"
done

echo "Full directory structure:"
find "$INSTALL_DIR" -type f | sort

# -----------------------------------
# COPY INTO vendor/prebuilt
# -----------------------------------
if [ -d "$PREBUILT_DIR" ]; then
    echo "Copying to vendor/prebuilt/mariadb-connector..."
    MARIADB_PREBUILT="$PREBUILT_DIR/mariadb-connector"
    rm -rf "$MARIADB_PREBUILT"
    mkdir -p "$MARIADB_PREBUILT"

    cp -r "$INSTALL_DIR/"* "$MARIADB_PREBUILT/"
    echo "Copied to prebuilt directory: $MARIADB_PREBUILT"
fi

echo "MariaDB connector build completed successfully!"
