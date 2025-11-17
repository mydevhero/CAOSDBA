#!/bin/bash
set -e

POSTGRES_SOURCE_DIR="$(cd $(dirname $0)/../postgres && pwd)"
BUILD_DIR="$(pwd)/postgres-build"
INSTALL_DIR="$(pwd)/postgres-install"
PREBUILT_DIR="$(cd $(dirname $0)/../prebuilt && pwd)"

echo "Building PostgreSQL static libraries..."
echo "• Source:  $POSTGRES_SOURCE_DIR"
echo "• Build:   $BUILD_DIR"
echo "• Install: $INSTALL_DIR"

rm -rf "$BUILD_DIR" "$INSTALL_DIR"
mkdir -p "$BUILD_DIR" "$INSTALL_DIR"

cd "$POSTGRES_SOURCE_DIR"

# -----------------------------------
# CONFIGURE
# -----------------------------------
CFLAGS="-fPIC" CXXFLAGS="-fPIC" ./configure \
  --prefix="$INSTALL_DIR" \
  --without-readline \
  --without-zlib \
  --without-openssl \
  --without-gssapi \
  --without-ldap \
  --without-pam \
  --without-icu \
  --enable-thread-safety

# -----------------------------------
# BUILD REQUIRED STATIC LIBS
# -----------------------------------

echo "Building libpgcommon..."
make -C src/common
make -C src/common libpgcommon_shlib.a

echo "Building libpgport..."
make -C src/port
make -C src/port libpgport_shlib.a

echo "Building libpq..."
make -C src/interfaces/libpq

# -----------------------------------
# INSTALL HEADERS + libpq
# -----------------------------------
echo "Installing libpq and headers..."
make -C src/interfaces/libpq install
make -C src/include install

# Copy required libs
mkdir -p "$INSTALL_DIR/lib"
cp src/common/libpgcommon.a "$INSTALL_DIR/lib/"
cp src/common/libpgcommon_shlib.a "$INSTALL_DIR/lib/"

cp src/port/libpgport.a "$INSTALL_DIR/lib/"
cp src/port/libpgport_shlib.a "$INSTALL_DIR/lib/"

cp src/interfaces/libpq/libpq.a "$INSTALL_DIR/lib/"

echo "PostgreSQL built successfully."

# -----------------------------------
# COPY INTO vendor/prebuilt
# -----------------------------------
if [ -d "$PREBUILT_DIR" ]; then
    echo "Copying to vendor/prebuilt/postgres..."
    POSTGRES_PREBUILT="$PREBUILT_DIR/postgres"
    mkdir -p "$POSTGRES_PREBUILT/lib" "$POSTGRES_PREBUILT/include"

    cp -r "$INSTALL_DIR/lib/"* "$POSTGRES_PREBUILT/lib/"
    cp -r "$INSTALL_DIR/include/"* "$POSTGRES_PREBUILT/include/"
fi

echo "Libraries generated:"
ls -la "$INSTALL_DIR/lib/"
