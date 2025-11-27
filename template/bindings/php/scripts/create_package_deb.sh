#!/bin/bash

# Get database backend and project name from command line
DB_BACKEND=${1:-"MYSQL"}
PROJECT_NAME=${2:-"caos"}  # Default to 'caos' for backward compatibility
PROJECT_NAME_SANITIZED=$(echo "$PROJECT_NAME" | tr '_' '-')
DB_BACKEND_LOWER=$(echo "$DB_BACKEND" | tr '[:upper:]' '[:lower:]')

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/release"
DIST_DIR="$PROJECT_ROOT/dist"
REPO_DIR="$DIST_DIR/repository"
PACKAGE_BASE_NAME="${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}"

if [ -f "$BUILD_DIR/build_counter.txt" ]; then
    CAOS_BUILD_COUNTER=$(cat "$BUILD_DIR/build_counter.txt")
    VERSION="1.0.0+${CAOS_BUILD_COUNTER}"
else
    VERSION="1.0.0+1"
    echo "WARNING: $BUILD_DIR/build_counter.txt not found, using default version"
fi

echo "Building DEB packages for ${PROJECT_NAME} - ALL PHP versions (8.0+)"
echo "Project: $PROJECT_NAME"
echo "Version: $VERSION"
echo "Database backend: $DB_BACKEND_LOWER"
echo "Output directory: $DIST_DIR"

PHP_VERSIONS="8.0 8.1 8.2 8.3 8.4"

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
    if [ ! -f "$BUILD_DIR/${PROJECT_NAME}.so" ]; then
        echo "ERROR: ${PROJECT_NAME}.so not found. Build the extension first at $BUILD_DIR/${PROJECT_NAME}.so"
        exit 1
    fi

    if [ ! -f "$BUILD_DIR/${PROJECT_NAME}.ini" ]; then
        echo "ERROR: ${PROJECT_NAME}.ini not found at $BUILD_DIR/${PROJECT_NAME}.ini"
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

    # Copy the extension .so file
    cp "$BUILD_DIR/${PROJECT_NAME}.so" "$deb_dir/$PHP_EXT_DIR/"

    # Copy the .ini configuration file
    cp "$BUILD_DIR/${PROJECT_NAME}.ini" "$deb_dir/$MODS_AVAILABLE_DIR/"

    echo "Files copied successfully:"
    echo "  - $deb_dir/$PHP_EXT_DIR/${PROJECT_NAME}.so"
    echo "  - $deb_dir/$MODS_AVAILABLE_DIR/${PROJECT_NAME}.ini"
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
PROJECT_NAME="$PROJECT_NAME"

echo ""
echo "Processing ${PROJECT_NAME} PHP extension for PHP \$PHP_VERSION"
echo ""

# Check if this PHP version is installed on target system
if [ ! -d "/etc/php/\$PHP_VERSION" ]; then
    echo "INFO: PHP \$PHP_VERSION not installed on this system"
    echo "Extension will be automatically enabled if PHP \$PHP_VERSION is installed later"
    exit 0
fi

echo "PHP \$PHP_VERSION is installed - enabling extension"

# Set owner and permissions
chown root:root "\$PHP_EXT_DIR/\$PROJECT_NAME.so" 2>/dev/null || true
chmod 644 "\$PHP_EXT_DIR/\$PROJECT_NAME.so" 2>/dev/null || true
chown root:root "\$MODS_AVAILABLE_DIR/\$PROJECT_NAME.ini" 2>/dev/null || true
chmod 644 "\$MODS_AVAILABLE_DIR/\$PROJECT_NAME.ini" 2>/dev/null || true

# Enable the extension
if command -v phpenmod >/dev/null 2>&1; then
    echo "Enabling \$PROJECT_NAME extension for PHP \$PHP_VERSION"
    phpenmod -v \$PHP_VERSION -s cli \$PROJECT_NAME 2>/dev/null && echo "Enabled for CLI" || echo "WARNING: Could not enable for CLI"
    phpenmod -v \$PHP_VERSION -s fpm \$PROJECT_NAME 2>/dev/null && echo "Enabled for FPM" || echo "WARNING: Could not enable for FPM"
    phpenmod -v \$PHP_VERSION -s apache2 \$PROJECT_NAME 2>/dev/null && echo "Enabled for Apache" || echo "WARNING: Could not enable for Apache"

    # Reload services if running
    systemctl reload "php\$PHP_VERSION-fpm" 2>/dev/null || true
    systemctl reload "apache2" 2>/dev/null || true
else
    echo "ERROR: phpenmod not found. Please install php\$PHP_VERSION-common"
fi

echo ""
echo "\$PROJECT_NAME PHP extension for PHP \$PHP_VERSION ready"
echo "TIP: Run: php\$PHP_VERSION -m | grep \$PROJECT_NAME  to verify"
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
PROJECT_NAME="$PROJECT_NAME"

echo ""
echo "Disabling \$PROJECT_NAME PHP extension for PHP \$PHP_VERSION before removal"
echo ""

# Disable extension for all SAPI if this PHP version is installed
if [ -d "/etc/php/\$PHP_VERSION" ] && command -v phpdismod >/dev/null 2>&1; then
    echo "Disabling extension for all SAPI"
    phpdismod -v \$PHP_VERSION -s cli \$PROJECT_NAME 2>/dev/null && echo "Disabled for CLI" || echo "WARNING: Could not disable for CLI"
    phpdismod -v \$PHP_VERSION -s fpm \$PROJECT_NAME 2>/dev/null && echo "Disabled for FPM" || echo "WARNING: Could not disable for FPM"
    phpdismod -v \$PHP_VERSION -s apache2 \$PROJECT_NAME 2>/dev/null && echo "Disabled for Apache" || echo "WARNING: Could not disable for Apache"

    # Reload services if running
    if systemctl is-active --quiet "php\$PHP_VERSION-fpm" 2>/dev/null; then
        echo "Reloading PHP-FPM"
        systemctl reload "php\$PHP_VERSION-fpm" 2>/dev/null || true
    fi

    if systemctl is-active --quiet "apache2" 2>/dev/null; then
        echo "Reloading Apache"
        systemctl reload "apache2" 2>/dev/null || true
    fi

    echo "\$PROJECT_NAME extension for PHP \$PHP_VERSION disabled before removal"
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

    # Common dependencies for all backends
    local common_depends="php${php_version}-common, libfmt-dev, libhiredis-dev, libspdlog-dev, ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}"

    case "${DB_BACKEND_LOWER}" in
        "mysql")
            common_depends="$common_depends, libmysqlclient-dev, libmysqlcppconn-dev"
            ;;
        "mariadb")
            # common_depends="$common_depends, libmariadb-dev"
            ;;
        "postgresql")
            # common_depends="$common_depends, libpq-dev, libpqxx-dev"
            ;;
        *)
            ;;
    esac

    # Generate conflicts based on project name and other backends
    local conflicts_clause=""
    case "${DB_BACKEND_LOWER}" in
        "mysql")
            conflicts_clause="${PROJECT_NAME_SANITIZED}-php-mariadb-${php_version}, ${PROJECT_NAME_SANITIZED}-php-postgresql-${php_version}"
            ;;
        "mariadb")
            conflicts_clause="${PROJECT_NAME_SANITIZED}-php-mysql-${php_version}, ${PROJECT_NAME_SANITIZED}-php-postgresql-${php_version}"
            ;;
        "postgresql")
            conflicts_clause="${PROJECT_NAME_SANITIZED}-php-mysql-${php_version}, ${PROJECT_NAME_SANITIZED}-php-mariadb-${php_version}"
            ;;
        *)
            ;;
    esac

    cat > "$deb_dir/DEBIAN/control" << EOF
Package: $package_name
Version: $VERSION
Architecture: $architecture
Maintainer: Alessandro Bianco <mydevhero@gmail.com>
Description: ${PROJECT_NAME^^} - Cache App On Steroids extension for PHP $php_version with $DB_BACKEND_LOWER backend
 High-performance database access extension (Cache App On Steroids).
 Supports: CLI, FPM, Apache, Nginx for PHP $php_version with $DB_BACKEND_LOWER backend.
Depends: $common_depends
Recommends: php$php_version-fpm | libapache2-mod-php$php_version
Conflicts: $conflicts_clause
Section: php
Priority: optional
EOF
}

build_single_package() {
    local php_version=$1
    local deb_dir="$BUILD_DIR/deb-staging-php$php_version"

    calculate_php_paths "$php_version"
    PHP_VERSION="$php_version"

    local package_name="${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-$php_version"

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

    # Common dependencies for meta-package
    local meta_common_depends="libfmt-dev, libhiredis-dev, libspdlog-dev"

    case "${DB_BACKEND_LOWER}" in
        "mysql")
            meta_common_depends="$meta_common_depends, libmysqlclient-dev, libmysqlcppconn-dev"
            ;;
        "mariadb")
            # meta_common_depends="$meta_common_depends, mariadb-common"
            ;;
        "postgresql")
            # meta_common_depends="$meta_common_depends, postgresql-common, libpq-dev, libpqxx-dev"
            ;;
        *)
            ;;
    esac

    # Meta-package recommends specific packages instead of depending on them
    local recommends_clause=""
    for version in $PHP_VERSIONS; do
        pkg="${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-$version"
        recommends_clause="$recommends_clause$pkg (>= $VERSION), "
    done
    recommends_clause="${recommends_clause%, }"

    # Generate conflicts for meta-package
    local meta_conflicts_clause=""
    case "${DB_BACKEND_LOWER}" in
        "mysql")
            meta_conflicts_clause="${PROJECT_NAME_SANITIZED}-php-mariadb, ${PROJECT_NAME_SANITIZED}-php-postgresql"
            ;;
        "mariadb")
            meta_conflicts_clause="${PROJECT_NAME_SANITIZED}-php-mysql, ${PROJECT_NAME_SANITIZED}-php-postgresql"
            ;;
        "postgresql")
            meta_conflicts_clause="${PROJECT_NAME_SANITIZED}-php-mysql, ${PROJECT_NAME_SANITIZED}-php-mariadb"
            ;;
        *)
            ;;
    esac

    cat > "$deb_dir/DEBIAN/control" << EOF
Package: ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}
Version: $VERSION
Architecture: all
Maintainer: Alessandro Bianco <mydevhero@gmail.com>
Description: ${PROJECT_NAME^^} - Cache App On Steroids PHP extension with $DB_BACKEND_LOWER backend (metapackage)
 High-performance database access extension (Cache App On Steroids).
 This metapackage will install the appropriate version for your PHP.
 Removing this metapackage will also remove the specific PHP version packages.
Depends: $meta_common_depends
Recommends: $recommends_clause
Conflicts: $meta_conflicts_clause
Section: php
Priority: optional
EOF

    echo "Building meta-package deb file..."
    dpkg-deb --build "$deb_dir" "$BUILD_DIR/${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-${VERSION}_all.deb"

    # Verify the file was created
    if [ -f "$BUILD_DIR/${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-${VERSION}_all.deb" ]; then
        echo "DEBUG: Meta-package successfully created: ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-${VERSION}_all.deb"
        ls -la "$BUILD_DIR/${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-${VERSION}_all.deb"
    else
        echo "DEBUG: ERROR - Meta-package file was not created!"
    fi

    rm -rf "$deb_dir"
    echo "Meta-package built: ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-${VERSION}_all.deb"
}

create_update_repo_script() {
    cat > "$REPO_DIR/update-repo.sh" << EOF
#!/bin/bash

REPO_DIR="\$(cd "\$(dirname "\$0")" && pwd)"
CONF_DIR="\$REPO_DIR/conf"
OVERRIDE_FILE="\$CONF_DIR/override"

echo "Updating ${PROJECT_NAME^^} repository index in \$REPO_DIR"

mkdir -p "\$CONF_DIR"

if [ ! -f "\$OVERRIDE_FILE" ]; then
    cat > "\$OVERRIDE_FILE" << OVERRIDE_CONTENT
${PROJECT_NAME_SANITIZED}-php-mysql optional php
${PROJECT_NAME_SANITIZED}-php-mysql-8.0 optional php
${PROJECT_NAME_SANITIZED}-php-mysql-8.1 optional php
${PROJECT_NAME_SANITIZED}-php-mysql-8.2 optional php
${PROJECT_NAME_SANITIZED}-php-mysql-8.3 optional php
${PROJECT_NAME_SANITIZED}-php-mysql-8.4 optional php
${PROJECT_NAME_SANITIZED}-php-mariadb optional php
${PROJECT_NAME_SANITIZED}-php-mariadb-8.0 optional php
${PROJECT_NAME_SANITIZED}-php-mariadb-8.1 optional php
${PROJECT_NAME_SANITIZED}-php-mariadb-8.2 optional php
${PROJECT_NAME_SANITIZED}-php-mariadb-8.3 optional php
${PROJECT_NAME_SANITIZED}-php-mariadb-8.4 optional php
${PROJECT_NAME_SANITIZED}-php-postgresql optional php
${PROJECT_NAME_SANITIZED}-php-postgresql-8.0 optional php
${PROJECT_NAME_SANITIZED}-php-postgresql-8.1 optional php
${PROJECT_NAME_SANITIZED}-php-postgresql-8.2 optional php
${PROJECT_NAME_SANITIZED}-php-postgresql-8.3 optional php
${PROJECT_NAME_SANITIZED}-php-postgresql-8.4 optional php
OVERRIDE_CONTENT
    echo "Created override file: \$OVERRIDE_FILE"
fi

cd "\$REPO_DIR" || exit 1

rm -f Release.gpg InRelease

dpkg-scanpackages --multiversion . "\$OVERRIDE_FILE" > Packages
gzip -k -f Packages

cat > Release << RELEASE_CONTENT
Origin: ${PROJECT_NAME^^} Repository
Label: ${PROJECT_NAME^^}
Suite: stable
Codename: ${PROJECT_NAME}
Version: 1.0
Architectures: amd64
Components: main
Description: ${PROJECT_NAME^^} - Cache App On Steroids PHP extension
Date: \$(date -Ru)
MD5Sum:
 \$(md5sum Packages | cut -d' ' -f1) \$(stat -c %s Packages) Packages
 \$(md5sum Packages.gz | cut -d' ' -f1) \$(stat -c %s Packages.gz) Packages.gz
SHA256:
 \$(sha256sum Packages | cut -d' ' -f1) \$(stat -c %s Packages) Packages
 \$(sha256sum Packages.gz | cut -d' ' -f1) \$(stat -c %s Packages.gz) Packages.gz
RELEASE_CONTENT

if [ \$? -eq 0 ]; then
    echo "Repository index updated successfully"
    echo ""
    echo "Packages in repository: \$(grep "^Package:" Packages | wc -l)"
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
    cat > "$REPO_DIR/setup-apt-source.sh" << EOF
#!/bin/bash

REPO_DIR="\$(cd "\$(dirname "\$0")" && pwd)"
APT_SOURCE_FILE="/etc/apt/sources.list.d/${PROJECT_NAME}-local.list"
APT_PREFERENCES_FILE="/etc/apt/preferences.d/${PROJECT_NAME}-local"

if [ "\$EUID" -ne 0 ]; then
    echo "ERROR: This script must be run as root (use sudo)"
    exit 1
fi

echo "Setting up ${PROJECT_NAME^^} local repository for APT"

if [ ! -d "\$REPO_DIR" ]; then
    echo "ERROR: Repository directory not found: \$REPO_DIR"
    exit 1
fi

if [ ! -f "\$REPO_DIR/Packages.gz" ]; then
    echo "ERROR: Repository index not found. Run: ./update-repo.sh first"
    exit 1
fi

echo "deb [trusted=yes] file:\$REPO_DIR ./" > "\$APT_SOURCE_FILE"

cat > "\$APT_PREFERENCES_FILE" << PREFERENCES
Package: *
Pin: origin ""
Pin-Priority: 1000

Package: ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}*
Pin: origin ""
Pin-Priority: 1001
PREFERENCES

if [ \$? -eq 0 ]; then
    echo "APT source configured: \$APT_SOURCE_FILE"
    echo "APT preferences configured: \$APT_PREFERENCES_FILE"
    echo ""
    echo "Updating APT cache"
    apt update
    echo ""
    echo "Setup completed"
    echo ""
    echo "Install ${PROJECT_NAME^^} PHP extension:"
    echo "sudo apt install ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}"
    echo ""
    echo "Or install for specific PHP version:"
    echo "sudo apt install ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-8.3"
else
    echo "ERROR: Failed to create APT configuration"
    exit 1
fi
EOF
    chmod +x "$REPO_DIR/setup-apt-source.sh"
    echo "Created: $REPO_DIR/setup-apt-source.sh"
}

create_install_repository_script() {
    cat > "$DIST_DIR/install-repository.sh" << EOF
#!/bin/bash

set -e

SCRIPT_DIR="\$(cd "\$(dirname "\$0")" && pwd)"
REPO_DIR="\$SCRIPT_DIR/repository"

if [ "\$EUID" -ne 0 ]; then
    echo "ERROR: This script must be run as root (use sudo)"
    exit 1
fi

echo "================================================================"
echo "${PROJECT_NAME^^} Repository Installation"
echo "================================================================"

if [ ! -d "\$REPO_DIR" ]; then
    echo "ERROR: Repository directory not found: \$REPO_DIR"
    echo "Make sure this script is in the same directory as the 'repository' folder"
    exit 1
fi

echo "Setting up ${PROJECT_NAME^^} repository from \$REPO_DIR"

echo "Updating repository index..."
cd "\$REPO_DIR"
if [ -f "./update-repo.sh" ]; then
    ./update-repo.sh
else
    echo "ERROR: update-repo.sh not found in \$REPO_DIR"
    exit 1
fi

echo "Configuring APT repository..."
if [ -f "./setup-apt-source.sh" ]; then
    ./setup-apt-source.sh
else
    echo "ERROR: setup-apt-source.sh not found in \$REPO_DIR"
    exit 1
fi

echo ""
echo "================================================================"
echo "${PROJECT_NAME^^} Repository Installation Completed"
echo "================================================================"
echo ""
echo "You can now install ${PROJECT_NAME^^} packages:"
echo "sudo apt install ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}"
echo ""
echo "Or for specific PHP version:"
echo "sudo apt install ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-8.3"
echo ""
echo "Repository location: \$REPO_DIR"
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
    echo "Starting DEB package creation for $PROJECT_NAME..."

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
