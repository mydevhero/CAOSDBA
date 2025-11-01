#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../../" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/release"
DEB_DIR="$BUILD_DIR/deb-staging"
PACKAGE_NAME="caos"
VERSION="1.0.0"

detect_php_variables() {
    if command -v php-config >/dev/null 2>&1; then
        PHP_VERSION=$(php-config --version | cut -d '.' -f 1-2)
        PHP_EXT_DIR=$(php-config --extension-dir)
        PHP_API=$(basename "$PHP_EXT_DIR")
    else
        echo "⚠️  php-config not found. Falling back to hardcoded 8.3/20230831."
        PHP_VERSION="8.3"
        PHP_API="20230831"
        PHP_EXT_DIR="/usr/lib/php/$PHP_API"
    fi

    MODS_AVAILABLE_DIR="/etc/php/$PHP_VERSION/mods-available"

    echo "🔍 Detected PHP version: $PHP_VERSION (API: $PHP_API)"
    echo "🔍 Extension dir: $PHP_EXT_DIR"
    echo "🔍 Mods available: $MODS_AVAILABLE_DIR"
}

echo "🎯 Building DEB package for CAOS PHP extension (All SAPI support)..."

check_prerequisites() {
    if [ ! -f "$BUILD_DIR/extensions/php/caos.so" ]; then
        echo "❌ caos.so not found. Build the extension first at $BUILD_DIR/extensions/php/caos.so"
        exit 1
    fi

    if ! command -v dpkg-deb >/dev/null 2>&1; then
        echo "❌ dpkg-deb not found. Install dpkg-dev package (sudo apt install dpkg-dev)."
        exit 1
    fi
}

create_structure() {
    echo "📁 Creating package structure for all PHP SAPI..."
    rm -rf "$DEB_DIR"
    mkdir -p "$DEB_DIR$PHP_EXT_DIR"
    mkdir -p "$DEB_DIR$MODS_AVAILABLE_DIR"
    mkdir -p "$DEB_DIR/DEBIAN"
}

copy_files() {
    echo "📦 Copying files..."

    cp "$BUILD_DIR/extensions/php/caos.so" "$DEB_DIR$PHP_EXT_DIR/"

    echo "extension=caos.so" > "$DEB_DIR$MODS_AVAILABLE_DIR/caos.ini"
}

create_postinst() {
echo "⚙️ Creating post-install script for all SAPI..."
cat > "$DEB_DIR/DEBIAN/postinst" << EOF
#!/bin/bash
# postinst
set -e

PHP_VERSION="$PHP_VERSION"
echo ""
echo "🔧 Enabling CAOS PHP extension for all SAPI (PHP \$PHP_VERSION)..."
echo ""

# CORREZIONE: Imposta owner e permessi corretti
chown root:root "$PHP_EXT_DIR/caos.so"
chmod 644 "$PHP_EXT_DIR/caos.so"
chown root:root "$MODS_AVAILABLE_DIR/caos.ini"
chmod 644 "$MODS_AVAILABLE_DIR/caos.ini"

# Funzione per abilitare e ricaricare servizi
enable_and_reload() {
    local sapi=\$1
    local service_name=\$2

    if command -v phpenmod >/dev/null 2>&1; then
        echo "Enabling for \$sapi..."
        phpenmod -v \$PHP_VERSION -s \$sapi caos 2>/dev/null && echo "✅ Enabled for \$sapi" || echo "⚠️  Could not enable for \$sapi (SAPI not installed?)"

        # Ricarica il servizio se è in esecuzione
        if [ -n "\$service_name" ] && systemctl is-active --quiet "\$service_name" 2>/dev/null; then
            echo "🔄 Reloading \$service_name..."
            systemctl reload "\$service_name" 2>/dev/null || true
        fi
    else
        echo "❌ phpenmod not found. Please install php$PHP_VERSION-common and run:"
        echo "   sudo ln -s $MODS_AVAILABLE_DIR/caos.ini /etc/php/\$PHP_VERSION/\$sapi/conf.d/20-caos.ini"
    fi
}

# Abilita per tutti i SAPI disponibili
enable_and_reload cli ""
enable_and_reload fpm "php\$PHP_VERSION-fpm"
enable_and_reload apache2 "apache2"

echo ""
echo "🎉 CAOS PHP extension installed successfully!"
echo ""
echo "📋 Quick verification:"
echo "   php -m | grep caos"
echo ""
EOF
    chmod +x "$DEB_DIR/DEBIAN/postinst"
}

create_prerm() {
echo "⚙️ Creating pre-remove script..."
cat > "$DEB_DIR/DEBIAN/prerm" << EOF
#!/bin/bash
# prerm
set -e

PHP_VERSION="$PHP_VERSION"

# Disabilita l'estensione prima della rimozione
if command -v phpdismod >/dev/null 2>&1; then
    echo "🔧 Disabling CAOS PHP extension for all SAPI (PHP \$PHP_VERSION)..."

    # Disabilita per tutti i SAPI
    phpdismod -v \$PHP_VERSION -s cli caos 2>/dev/null || true
    phpdismod -v \$PHP_VERSION -s fpm caos 2>/dev/null || true
    phpdismod -v \$PHP_VERSION -s apache2 caos 2>/dev/null || true

    # Ricarica i servizi
    systemctl reload "php\$PHP_VERSION-fpm" 2>/dev/null || true
    systemctl reload "apache2" 2>/dev/null || true

    echo "✅ CAOS extension disabled."
fi
EOF
    chmod +x "$DEB_DIR/DEBIAN/prerm"
}

create_control() {
echo "📝 Creating control file..."
cat > "$DEB_DIR/DEBIAN/control" << EOF
Package: $PACKAGE_NAME
Version: $VERSION
Architecture: amd64
Maintainer: Alessandro Bianco <mydevhero@gmail.com>
Description: CAOS - Extension for PHP $PHP_VERSION
 High-performance database access extension (Cache App On Steroids).
 Supports: CLI, FPM, Apache, Nginx
Depends: php$PHP_VERSION-common
Recommends: php$PHP_VERSION-fpm | libapache2-mod-php$PHP_VERSION
Section: php
Priority: optional
EOF
}

build_package() {
    echo "🏗️ Building DEB package..."
    dpkg-deb --build "$DEB_DIR" "$BUILD_DIR/$PACKAGE_NAME-$VERSION-php$PHP_VERSION.deb"

    # Clean
    rm -rf "$DEB_DIR"

    echo "✅ DEB package created: /home/mrbi314/qt-project/caos/build/release/caos-1.0.0-php8.3.deb"
}

verify_package() {
    echo "🔍 Verifying package contents..."
    dpkg -c "$BUILD_DIR/$PACKAGE_NAME-$VERSION-php$PHP_VERSION.deb"
    echo ""
    echo "📦 Package contents:"
    echo "   - $PHP_EXT_DIR/caos.so"
    echo "   - $MODS_AVAILABLE_DIR/caos.ini"
}

main() {
    detect_php_variables
    check_prerequisites
    create_structure
    copy_files
    create_postinst
    create_prerm
    create_control
    build_package
    verify_package
}

main "$@"
