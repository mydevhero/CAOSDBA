#!/bin/bash
set -e

POSTGRES_SOURCE_DIR="$(cd $(dirname $0)/../postgres && pwd)"
BUILD_DIR="$(pwd)/postgres-build"
INSTALL_DIR="$(pwd)/postgres-install"
PREBUILT_DIR="$(cd $(dirname $0)/../prebuilt && pwd)"

echo "Building PostgreSQL static libraries..."
echo "• Source: $POSTGRES_SOURCE_DIR"
echo "• Build: $BUILD_DIR"
echo "• Install: $INSTALL_DIR"

# Pulisci e crea directories
rm -rf $BUILD_DIR $INSTALL_DIR
mkdir -p $BUILD_DIR $INSTALL_DIR

cd $POSTGRES_SOURCE_DIR

# Configure
CFLAGS="-fPIC" CXXFLAGS="-fPIC" ./configure \
  --prefix=$INSTALL_DIR \
  --without-readline --without-zlib --without-openssl \
  --without-gssapi --without-ldap --without-pam \
  --without-icu --enable-thread-safety

# Build solo le librerie necessarie
echo "Building libpgcommon..."
CFLAGS="-fPIC" CXXFLAGS="-fPIC" make -C src/common

echo "Building libpgport..."
CFLAGS="-fPIC" CXXFLAGS="-fPIC" make -C src/port

echo "Building libpq..."
CFLAGS="-fPIC" CXXFLAGS="-fPIC" make -C src/interfaces/libpq

# Install
echo "Installing libraries..."
make -C src/interfaces/libpq install
make -C src/include install

# Copia manualmente le librerie statiche che configure non installa
cp src/common/libpgcommon.a $INSTALL_DIR/lib/
cp src/port/libpgport.a $INSTALL_DIR/lib/

echo "PostgreSQL built successfully in $INSTALL_DIR"

if [ -d "$PREBUILT_DIR" ]; then
    echo "Copying to vendor/prebuilt..."

    POSTGRES_PREBUILT="$PREBUILT_DIR/postgres"
    mkdir -p $POSTGRES_PREBUILT/lib $POSTGRES_PREBUILT/include

    cp -r $INSTALL_DIR/lib/* $POSTGRES_PREBUILT/lib/
    cp -r $INSTALL_DIR/include/* $POSTGRES_PREBUILT/include/

    echo "Copied to vendor/prebuilt/postgres"
else
    echo "vendor/prebuilt directory not found, skipping copy"
fi

echo "Libraries:"
ls -la $INSTALL_DIR/lib/
