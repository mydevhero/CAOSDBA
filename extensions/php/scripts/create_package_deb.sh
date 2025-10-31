#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../../" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/release"
DEB_DIR="$BUILD_DIR/deb-staging"
PACKAGE_NAME="caos-php"
VERSION="1.0.0"

PHP_VERSION="8.3"
PHP_API="20230831"
INSTALL_DIR="/usr/lib/php/$PHP_API"
CONF_DIR="/etc/php/$PHP_VERSION/mods-available"

echo "🎯 Building DEB package for CAOS PHP extension..."


check_prerequisites() {
    if [ ! -f "$BUILD_DIR/extensions/php/caos_php.so" ]; then
        echo "❌ caos_php.so not found. Build the extension first."
        exit 1
    fi

    if ! command -v dpkg-deb >/dev/null 2>&1; then
        echo "❌ dpkg-deb not found. Install dpkg-dev package."
        exit 1
    fi
}

create_structure() {
    echo "📁 Creating package structure..."
    rm -rf "$DEB_DIR"
    mkdir -p "$DEB_DIR$INSTALL_DIR"
    mkdir -p "$DEB_DIR$CONF_DIR"
    mkdir -p "$DEB_DIR/DEBIAN"
}


copy_files() {
    echo "📦 Copying files..."
    cp "$BUILD_DIR/extensions/php/caos_php.so" "$DEB_DIR$INSTALL_DIR/"
    echo "extension=caos_php.so" > "$DEB_DIR$CONF_DIR/caos.ini"
}

create_postinst() {
echo "⚙️ Creating post-install script..."
cat > "$DEB_DIR/DEBIAN/postinst" << 'EOF'
#!/bin/bash
set -e
echo "🔧 Enabling CAOS PHP extension..."
if command -v phpenmod >/dev/null 2>&1; then
    phpenmod caos
    echo "✅ CAOS extension enabled via phpenmod"
else
    echo "💡 Manual: Add 'extension=caos_php.so' to php.ini"
fi
echo "🎉 CAOS PHP extension installed successfully!"
EOF
    chmod +x "$DEB_DIR/DEBIAN/postinst"
}

create_control() {
echo "📝 Creating control file..."
cat > "$DEB_DIR/DEBIAN/control" << EOF
Package: $PACKAGE_NAME
Version: $VERSION
Architecture: amd64
Maintainer: Alessandro Bianco <mydevhero@gmail.com>
Description: CAOS - Extension for PHP
 Cache App On Steroids: High-performance database access extension.
Depends: php$PHP_VERSION
Section: php
Priority: optional
EOF
}

build_package() {
    echo "🏗️ Building DEB package..."
    dpkg-deb --build "$DEB_DIR" "$BUILD_DIR/$PACKAGE_NAME-$VERSION.deb"

    # Clean
    rm -rf "$DEB_DIR"

    echo "✅ DEB package created: $BUILD_DIR/$PACKAGE_NAME-$VERSION.deb"
}

verify_package() {
    echo "🔍 Verifying package contents..."
    dpkg -c "$BUILD_DIR/$PACKAGE_NAME-$VERSION.deb"
}

main() {
    check_prerequisites
    create_structure
    copy_files
    create_postinst
    create_control
    build_package
    verify_package
}

main "$@"
