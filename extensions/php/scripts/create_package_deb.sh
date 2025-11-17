#!/bin/bash

# Get database backend from command line or use default
DB_BACKEND=${1:-"MYSQL"}
DB_BACKEND_LOWER=$(echo "$DB_BACKEND" | tr '[:upper:]' '[:lower:]')

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../../" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/release"
DIST_DIR="$PROJECT_ROOT/dist"
REPO_DIR="$DIST_DIR/repository"
PACKAGE_BASE_NAME="caos-php-${DB_BACKEND_LOWER}"

if [ -f "$BUILD_DIR/build_counter.txt" ]; then
    CAOS_BUILD_COUNTER=$(cat "$BUILD_DIR/build_counter.txt")
    VERSION="1.0.0+${CAOS_BUILD_COUNTER}"
else
    VERSION="1.0.0+1"
    echo "WARNING: $BUILD_DIR/build_counter.txt not found, using default version"
fi

PHP_VERSIONS="8.0 8.1 8.2 8.3 8.4"

echo "Building DEB packages for ALL PHP versions (8.0+)"
echo "Version: $VERSION"
echo "Database backend: $DB_BACKEND"
echo "Output directory: $DIST_DIR"

get_php_api_version() {
    local php_version=$1
    case $php_version in
        "8.0") echo "20200930" ;;
        "8.1") echo "20210902" ;;
        "8.2") echo "20220829" ;;
        "8.3") echo "20230831" ;;
        "8.4") echo "20240829" ;;
        *) echo "20230831" ;;
    esac
}

calculate_php_paths() {
    local php_version=$1
    PHP_API=$(get_php_api_version "$php_version")
    PHP_EXT_DIR="/usr/lib/php/$PHP_API"
    MODS_AVAILABLE_DIR="/etc/php/$php_version/mods-available"

    echo "Config for PHP $php_version:"
    echo "   API: $PHP_API"
    echo "   Extension dir: $PHP_EXT_DIR"
    echo "   Mods available: $MODS_AVAILABLE_DIR"
}

check_prerequisites() {
    if [ ! -f "$BUILD_DIR/extensions/php/caos.so" ]; then
        echo "ERROR: caos.so not found. Build the extension first at $BUILD_DIR/extensions/php/caos.so"
        exit 1
    fi

    if ! command -v dpkg-deb >/dev/null 2>&1; then
        echo "ERROR: dpkg-deb not found. Install dpkg-dev package: sudo apt install dpkg-dev"
        exit 1
    fi
}

create_structure() {
    local deb_dir=$1
    rm -rf "$deb_dir"
    mkdir -p "$deb_dir/$PHP_EXT_DIR"
    mkdir -p "$deb_dir/$MODS_AVAILABLE_DIR"
    mkdir -p "$deb_dir/DEBIAN"
}

copy_files() {
    local deb_dir=$1
    echo "Copying files for PHP $PHP_VERSION"

    cp "$BUILD_DIR/extensions/php/caos.so" "$deb_dir/$PHP_EXT_DIR/"
    echo "extension=caos.so" > "$deb_dir/$MODS_AVAILABLE_DIR/caos.ini"
    echo "Files copied successfully"
}

create_postinst() {
    local deb_dir=$1
    local php_version=$2
    cat > "$deb_dir/DEBIAN/postinst" << EOF
#!/bin/bash
# postinst
set -e

PHP_VERSION="$php_version"
PHP_EXT_DIR="$PHP_EXT_DIR"
MODS_AVAILABLE_DIR="$MODS_AVAILABLE_DIR"

echo ""
echo "Processing CAOS PHP extension for PHP \$PHP_VERSION"
echo ""

# Check if this PHP version is installed on target system
if [ ! -d "/etc/php/\$PHP_VERSION" ]; then
    echo "INFO: PHP \$PHP_VERSION not installed on this system"
    echo "Extension will be automatically enabled if PHP \$PHP_VERSION is installed later"
    exit 0
fi

echo "PHP \$PHP_VERSION is installed - enabling extension"

# Set owner and permissions
chown root:root "\$PHP_EXT_DIR/caos.so" 2>/dev/null || true
chmod 644 "\$PHP_EXT_DIR/caos.so" 2>/dev/null || true
chown root:root "\$MODS_AVAILABLE_DIR/caos.ini" 2>/dev/null || true
chmod 644 "\$MODS_AVAILABLE_DIR/caos.ini" 2>/dev/null || true

# Enable the extension
if command -v phpenmod >/dev/null 2>&1; then
    echo "Enabling CAOS extension for PHP \$PHP_VERSION"
    phpenmod -v \$PHP_VERSION -s cli caos 2>/dev/null && echo "Enabled for CLI" || echo "WARNING: Could not enable for CLI"
    phpenmod -v \$PHP_VERSION -s fpm caos 2>/dev/null && echo "Enabled for FPM" || echo "WARNING: Could not enable for FPM"
    phpenmod -v \$PHP_VERSION -s apache2 caos 2>/dev/null && echo "Enabled for Apache" || echo "WARNING: Could not enable for Apache"

    # Reload services if running
    systemctl reload "php\$PHP_VERSION-fpm" 2>/dev/null || true
    systemctl reload "apache2" 2>/dev/null || true
else
    echo "ERROR: phpenmod not found. Please install php\$PHP_VERSION-common"
fi

echo ""
echo "CAOS PHP extension for PHP \$PHP_VERSION ready"
echo "TIP: Run: php\$PHP_VERSION -m | grep caos  to verify"
echo ""
EOF
    chmod +x "$deb_dir/DEBIAN/postinst"
}

create_prerm() {
    local deb_dir=$1
    local php_version=$2
    cat > "$deb_dir/DEBIAN/prerm" << EOF
#!/bin/bash
# prerm
set -e

PHP_VERSION="$php_version"
PHP_EXT_DIR="$PHP_EXT_DIR"
MODS_AVAILABLE_DIR="$MODS_AVAILABLE_DIR"

echo ""
echo "Disabling CAOS PHP extension for PHP \$PHP_VERSION before removal"
echo ""

# Disable extension for all SAPI if this PHP version is installed
if [ -d "/etc/php/\$PHP_VERSION" ] && command -v phpdismod >/dev/null 2>&1; then
    echo "Disabling extension for all SAPI"
    phpdismod -v \$PHP_VERSION -s cli caos 2>/dev/null && echo "Disabled for CLI" || echo "WARNING: Could not disable for CLI"
    phpdismod -v \$PHP_VERSION -s fpm caos 2>/dev/null && echo "Disabled for FPM" || echo "WARNING: Could not disable for FPM"
    phpdismod -v \$PHP_VERSION -s apache2 caos 2>/dev/null && echo "Disabled for Apache" || echo "WARNING: Could not disable for Apache"

    # Reload services if running
    if systemctl is-active --quiet "php\$PHP_VERSION-fpm" 2>/dev/null; then
        echo "Reloading PHP-FPM"
        systemctl reload "php\$PHP_VERSION-fpm" 2>/dev/null || true
    fi

    if systemctl is-active --quiet "apache2" 2>/dev/null; then
        echo "Reloading Apache"
        systemctl reload "apache2" 2>/dev/null || true
    fi

    echo "CAOS extension for PHP \$PHP_VERSION disabled before removal"
else
    echo "INFO: PHP \$PHP_VERSION not installed or phpdismod not available"
fi

echo ""
EOF
    chmod +x "$deb_dir/DEBIAN/prerm"
}

create_control() {
    local deb_dir=$1
    local package_name=$2
    local php_version=$3
    local architecture="amd64"

    cat > "$deb_dir/DEBIAN/control" << EOF
Package: $package_name
Version: $VERSION
Architecture: $architecture
Maintainer: Alessandro Bianco <mydevhero@gmail.com>
Description: CAOS - Cache App On Steroids extension for PHP $php_version with $DB_BACKEND backend
 High-performance database access extension (Cache App On Steroids).
 Supports: CLI, FPM, Apache, Nginx for PHP $php_version with $DB_BACKEND backend.
Depends: php$php_version-common, libfmt-dev, libmysqlclient-dev, libmysqlcppconn-dev, libhiredis-dev, libspdlog-dev, caos-php-${DB_BACKEND_LOWER}
Recommends: php$php_version-fpm | libapache2-mod-php$php_version
Section: php
Priority: optional
EOF
}

build_single_package() {
    local php_version=$1
    local deb_dir="$BUILD_DIR/deb-staging-php$php_version"

    calculate_php_paths "$php_version"
    PHP_VERSION="$php_version"

    local package_name="caos-php-${DB_BACKEND_LOWER}-$php_version"

    echo "Building package: $package_name"

    create_structure "$deb_dir"
    if ! copy_files "$deb_dir"; then
        echo "ERROR: Failed to copy files for PHP $php_version"
        return 1
    fi
    create_postinst "$deb_dir" "$php_version"
    create_prerm "$deb_dir" "$php_version"
    create_control "$deb_dir" "$package_name" "$php_version"

    dpkg-deb --build "$deb_dir" "$BUILD_DIR/${package_name}_${VERSION}_amd64.deb"

    rm -rf "$deb_dir"

    echo "Built: ${package_name}_${VERSION}_amd64.deb"
    return 0
}

create_meta_package() {
    local deb_dir="$BUILD_DIR/deb-staging-meta"
    mkdir -p "$deb_dir/DEBIAN"

    echo "Creating meta-package with version: $VERSION"
    echo "Build counter: $CAOS_BUILD_COUNTER"

    # Il meta-pacchetto raccomanda i pacchetti specifici invece di dipendere da loro
    local recommends_clause=""
    for version in $PHP_VERSIONS; do
        pkg="caos-php-${DB_BACKEND_LOWER}-$version"
        recommends_clause="$recommends_clause$pkg (>= $VERSION), "
    done
    recommends_clause="${recommends_clause%, }"

    cat > "$deb_dir/DEBIAN/control" << EOF
Package: caos-php-${DB_BACKEND_LOWER}
Version: $VERSION
Architecture: all
Maintainer: Alessandro Bianco <mydevhero@gmail.com>
Description: CAOS - Cache App On Steroids PHP extension with $DB_BACKEND backend (metapackage)
 High-performance database access extension (Cache App On Steroids).
 This metapackage will install the appropriate version for your PHP.
 Removing this metapackage will also remove the specific PHP version packages.
Depends: ${DB_BACKEND_LOWER}-common
Recommends: $recommends_clause
Section: php
Priority: optional
EOF

    echo "Building meta-package deb file..."
    dpkg-deb --build "$deb_dir" "$BUILD_DIR/caos-php-${DB_BACKEND_LOWER}_${VERSION}_all.deb"

    # Verifica che il file sia stato creato
    if [ -f "$BUILD_DIR/caos-php-${DB_BACKEND_LOWER}_${VERSION}_all.deb" ]; then
        echo "DEBUG: Meta-package successfully created: caos-php-${DB_BACKEND_LOWER}_${VERSION}_all.deb"
        ls -la "$BUILD_DIR/caos-php-${DB_BACKEND_LOWER}_${VERSION}_all.deb"
    else
        echo "DEBUG: ERROR - Meta-package file was not created!"
    fi

    rm -rf "$deb_dir"
    echo "Meta-package built: caos-php-${DB_BACKEND_LOWER}_${VERSION}_all.deb"
}

create_update_repo_script() {
    cat > "$REPO_DIR/update-repo.sh" << 'EOF'
#!/bin/bash

REPO_DIR="$(cd "$(dirname "$0")" && pwd)"
CONF_DIR="$REPO_DIR/conf"
OVERRIDE_FILE="$CONF_DIR/override"

echo "Updating CAOS repository index in $REPO_DIR"

mkdir -p "$CONF_DIR"

if [ ! -f "$OVERRIDE_FILE" ]; then
    cat > "$OVERRIDE_FILE" << OVERRIDE_CONTENT
caos-php-mysql optional php
caos-php-mysql-8.0 optional php
caos-php-mysql-8.1 optional php
caos-php-mysql-8.2 optional php
caos-php-mysql-8.3 optional php
caos-php-mysql-8.4 optional php
OVERRIDE_CONTENT
    echo "Created override file: $OVERRIDE_FILE"
fi

cd "$REPO_DIR" || exit 1

rm -f Release.gpg InRelease

dpkg-scanpackages --multiversion . "$OVERRIDE_FILE" > Packages
gzip -k -f Packages

cat > Release << RELEASE_CONTENT
Origin: CAOS Repository
Label: CAOS
Suite: stable
Codename: caos
Version: 1.0
Architectures: amd64
Components: main
Description: CAOS - Cache App On Steroids PHP extension
Date: $(date -Ru)
MD5Sum:
 $(md5sum Packages | cut -d' ' -f1) $(stat -c %s Packages) Packages
 $(md5sum Packages.gz | cut -d' ' -f1) $(stat -c %s Packages.gz) Packages.gz
SHA256:
 $(sha256sum Packages | cut -d' ' -f1) $(stat -c %s Packages) Packages
 $(sha256sum Packages.gz | cut -d' ' -f1) $(stat -c %s Packages.gz) Packages.gz
RELEASE_CONTENT

if [ $? -eq 0 ]; then
    echo "Repository index updated successfully"
    echo ""
    echo "Packages in repository: $(grep "^Package:" Packages | wc -l)"
    echo ""
    echo "Now run: sudo apt update"
else
    echo "ERROR: Failed to update repository index"
    exit 1
fi
EOF
    chmod +x "$REPO_DIR/update-repo.sh"
    echo "Created: $REPO_DIR/update-repo.sh"
}

create_setup_repo_script() {
    cat > "$REPO_DIR/setup-apt-source.sh" << 'EOF'
#!/bin/bash

REPO_DIR="$(cd "$(dirname "$0")" && pwd)"
APT_SOURCE_FILE="/etc/apt/sources.list.d/caos-local.list"
APT_PREFERENCES_FILE="/etc/apt/preferences.d/caos-local"

if [ "$EUID" -ne 0 ]; then
    echo "ERROR: This script must be run as root (use sudo)"
    exit 1
fi

echo "Setting up CAOS local repository for APT"

if [ ! -d "$REPO_DIR" ]; then
    echo "ERROR: Repository directory not found: $REPO_DIR"
    exit 1
fi

if [ ! -f "$REPO_DIR/Packages.gz" ]; then
    echo "ERROR: Repository index not found. Run: ./update-repo.sh first"
    exit 1
fi

echo "deb [trusted=yes] file:$REPO_DIR ./" > "$APT_SOURCE_FILE"

cat > "$APT_PREFERENCES_FILE" << PREFERENCES
Package: *
Pin: origin ""
Pin-Priority: 1000

Package: caos-php-mysql*
Pin: origin ""
Pin-Priority: 1001
PREFERENCES

if [ $? -eq 0 ]; then
    echo "APT source configured: $APT_SOURCE_FILE"
    echo "APT preferences configured: $APT_PREFERENCES_FILE"
    echo ""
    echo "Updating APT cache"
    apt update
    echo ""
    echo "Setup completed"
    echo ""
    echo "Install CAOS PHP extension:"
    echo "sudo apt install caos-php-mysql"
    echo ""
    echo "Or install for specific PHP version:"
    echo "sudo apt install caos-php-mysql-8.3"
else
    echo "ERROR: Failed to create APT configuration"
    exit 1
fi
EOF
    chmod +x "$REPO_DIR/setup-apt-source.sh"
    echo "Created: $REPO_DIR/setup-apt-source.sh"
}

create_install_repository_script() {
    cat > "$DIST_DIR/install-repository.sh" << 'EOF'
#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$SCRIPT_DIR/repository"

if [ "$EUID" -ne 0 ]; then
    echo "ERROR: This script must be run as root (use sudo)"
    exit 1
fi

echo "================================================================"
echo "CAOS Repository Installation"
echo "================================================================"

if [ ! -d "$REPO_DIR" ]; then
    echo "ERROR: Repository directory not found: $REPO_DIR"
    echo "Make sure this script is in the same directory as the 'repository' folder"
    exit 1
fi

echo "Setting up CAOS repository from $REPO_DIR"

echo "Updating repository index..."
cd "$REPO_DIR"
if [ -f "./update-repo.sh" ]; then
    ./update-repo.sh
else
    echo "ERROR: update-repo.sh not found in $REPO_DIR"
    exit 1
fi

echo "Configuring APT repository..."
if [ -f "./setup-apt-source.sh" ]; then
    ./setup-apt-source.sh
else
    echo "ERROR: setup-apt-source.sh not found in $REPO_DIR"
    exit 1
fi

echo ""
echo "================================================================"
echo "CAOS Repository Installation Completed"
echo "================================================================"
echo ""
echo "You can now install CAOS packages:"
echo "sudo apt install caos-php-mysql"
echo ""
echo "Or for specific PHP version:"
echo "sudo apt install caos-php-mysql-8.3"
echo ""
echo "Repository location: $REPO_DIR"
echo ""
EOF

    chmod +x "$DIST_DIR/install-repository.sh"
    echo "Created: $DIST_DIR/install-repository.sh"
}

move_packages_to_repository() {
    echo ""
    echo "Moving packages to repository directory: $REPO_DIR"
    mkdir -p "$REPO_DIR"

    mv "$BUILD_DIR"/*.deb "$REPO_DIR/" 2>/dev/null || true

    local moved_count=$(ls -1 "$REPO_DIR"/*.deb 2>/dev/null | wc -l)
    echo "Moved $moved_count package(s) to $REPO_DIR"

    echo ""
    echo "Packages created:"
    for pkg in "$REPO_DIR"/*.deb; do
        if [ -f "$pkg" ]; then
            echo "  $(basename "$pkg")"
        fi
    done
}

main() {
    echo "Starting DEB package creation..."

    check_prerequisites

    echo "Building packages for PHP versions: $PHP_VERSIONS"
    echo ""

    local built_count=0
    for version in $PHP_VERSIONS; do
        echo "=== PROCESSING PHP $version ==="
        if build_single_package "$version"; then
            built_count=$((built_count + 1))
            echo "SUCCESS: PHP $version package built"
        else
            echo "WARNING: PHP $version package skipped"
        fi
        echo "=== COMPLETED PHP $version ==="
        echo ""
    done

    if [ $built_count -gt 0 ]; then
        echo "=== CREATING META PACKAGE ==="
        create_meta_package
        echo ""

        move_packages_to_repository

        echo "=== CREATING REPOSITORY SCRIPTS ==="
        create_update_repo_script
        create_setup_repo_script
        create_install_repository_script

        echo "Multi-version packaging completed"
        echo "Built $built_count individual packages + 1 meta-package"
        echo "Repository created in: $REPO_DIR"
        exit 0
    else
        echo "ERROR: No packages were built successfully"
        exit 1
    fi
}

main "$@"
