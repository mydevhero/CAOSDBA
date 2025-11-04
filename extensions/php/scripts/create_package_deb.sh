#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../../" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/release"
PACKAGE_BASE_NAME="caos-php"

if [ -f "$BUILD_DIR/libcaos/build_counter.txt" ]; then
    CAOS_BUILD_COUNTER=$(cat "$BUILD_DIR/libcaos/build_counter.txt")
    VERSION="1.0.0+${CAOS_BUILD_COUNTER}"
else
    VERSION="1.0.0+1"
    echo "WARNING: $BUILD_DIR/libcaos/build_counter.txt not found, using default version"
fi

REPO_DIR="/opt/caos/repository"

# Fixed list of all target PHP versions (>=8.0)
PHP_VERSIONS=("8.0" "8.1" "8.2" "8.3" "8.4")
META_PACKAGES=()

echo "Building DEB packages for ALL PHP versions (8.0+)"
echo "Version: $VERSION"

# Map PHP versions to API version (e.g.: 8.3 -> 20230831)
get_php_api_version() {
    local php_version=$1
    case $php_version in
        "8.0") echo "20200930" ;;
        "8.1") echo "20210902" ;;
        "8.2") echo "20220829" ;;
        "8.3") echo "20230831" ;;
        "8.4") echo "20240829" ;;  # Placeholder for future
        *) echo "20230831" ;;      # Fallback
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

    if ! command -v dpkg-scanpackages >/dev/null 2>&1; then
        echo "ERROR: dpkg-scanpackages not found. Install dpkg-dev package: sudo apt install dpkg-dev"
        exit 1
    fi
}

check_repository_directory() {
    echo ""
    echo "Checking repository directory: $REPO_DIR"
    echo ""

    if [ ! -d "$REPO_DIR" ]; then
        echo "ERROR: Repository directory does not exist: $REPO_DIR"
        echo ""
        echo "Please create it first with:"
        echo ""
        echo "  sudo mkdir -p $REPO_DIR"
        echo "  sudo chown $USER:$USER $REPO_DIR"
        echo "  sudo chmod 755 $REPO_DIR"
        echo ""
        echo "Then run this script again."
        echo ""
        exit 1
    fi

    if [ ! -w "$REPO_DIR" ]; then
        echo "ERROR: Repository directory is not writable: $REPO_DIR"
        echo ""
        echo "Please fix permissions with:"
        echo ""
        echo "  sudo chown $USER:$USER $REPO_DIR"
        echo ""
        exit 1
    fi

    echo "Repository directory OK: $REPO_DIR"
    echo ""
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

    # Use the same binary for all versions
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

# Set owner and permissions (handled automatically by dpkg, but just in case)
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
Description: CAOS - Cache App On Steroids extension for PHP $php_version
 High-performance database access extension (Cache App On Steroids).
 Supports: CLI, FPM, Apache, Nginx for PHP $php_version
Depends: php$php_version-common
Recommends: php$php_version-fpm | libapache2-mod-php$php_version
Section: php
Priority: optional
EOF
}

build_single_package() {
    local php_version=$1
    local deb_dir="$BUILD_DIR/deb-staging-php$php_version"

    # Calculate paths for this PHP version
    calculate_php_paths "$php_version"
    PHP_VERSION="$php_version"

    local package_name="caos-php-$php_version"

    echo "Building package: $package_name"

    create_structure "$deb_dir"
    if ! copy_files "$deb_dir"; then
        echo "ERROR: Failed to copy files for PHP $php_version"
        return 1
    fi
    create_postinst "$deb_dir" "$php_version"
    create_prerm "$deb_dir" "$php_version"
    create_control "$deb_dir" "$package_name" "$php_version"

    # Build package
    dpkg-deb --build "$deb_dir" "$BUILD_DIR/${package_name}_${VERSION}_amd64.deb"

    # Cleanup
    rm -rf "$deb_dir"

    echo "Built: ${package_name}_${VERSION}_amd64.deb"
    META_PACKAGES+=("$package_name")
    return 0
}

create_meta_package() {
    if [ ${#META_PACKAGES[@]} -eq 0 ]; then
        echo "WARNING: No version-specific packages built"
        return 0
    fi

    local deb_dir="$BUILD_DIR/deb-staging-meta"
    mkdir -p "$deb_dir/DEBIAN"

    # Create OR dependencies for all versions
    local depends_clause=""
    for pkg in "${META_PACKAGES[@]}"; do
        depends_clause+="$pkg (>= $VERSION) | "
    done
    depends_clause="${depends_clause% | }"

    cat > "$deb_dir/DEBIAN/control" << EOF
Package: caos-php
Version: $VERSION
Architecture: all
Maintainer: Alessandro Bianco <mydevhero@gmail.com>
Description: CAOS - Cache App On Steroids PHP extension (metapackage)
 High-performance database access extension (Cache App On Steroids).
 Automatically selects the correct version for your PHP installation.
Depends: $depends_clause
Section: php
Priority: optional
EOF

    dpkg-deb --build "$deb_dir" "$BUILD_DIR/caos-php_${VERSION}_all.deb"
    rm -rf "$deb_dir"

    echo "Meta-package built: caos-php_${VERSION}_all.deb"
    echo "Depends on: ${META_PACKAGES[*]}"
}

setup_local_repository() {
    echo ""
    echo "Setting up local repository in $REPO_DIR"

    # Copy all .deb files to repository
    echo "Copying .deb packages to repository"
    cp "$BUILD_DIR"/*.deb "$REPO_DIR/" 2>/dev/null || true

    local copied_count=$(ls -1 "$REPO_DIR"/*.deb 2>/dev/null | wc -l)
    echo "Copied $copied_count package(s)"

    # Generate repository index
    echo "Generating repository index"
    cd "$REPO_DIR"

    # Create directory conf and override file
    mkdir -p "$REPO_DIR/conf"
    if [ ! -f "$REPO_DIR/conf/override" ]; then
        cat > "$REPO_DIR/conf/override" << EOF
caos-php optional php
caos-php-8.0 optional php
caos-php-8.1 optional php
caos-php-8.2 optional php
caos-php-8.3 optional php
caos-php-8.4 optional php
EOF
        echo "Created override file: $REPO_DIR/conf/override"
    fi

    # Use dpkg-scanpackages with override file
    dpkg-scanpackages --multiversion . "$REPO_DIR/conf/override" > Packages
    gzip -k -f Packages

    # Create complete Release file
    cat > Release << EOF
Origin: CAOS Repository
Label: CAOS
Suite: stable
Codename: caos
Version: 1.0
Architectures: amd64
Components: main
Description: CAOS - Cache App On Steroids PHP extension
Date: $(date -Ru)
Acquire-By-Hash: no
No-Support-for-Architecture-all: false
EOF

    # Create empty Release.gpg file (signed optional)
    touch Release.gpg

    # Create empty translation files
    mkdir -p "$REPO_DIR/i18n"
    for lang in it_IT en_US en it; do
        echo "# Empty translation file" > "$REPO_DIR/i18n/Translation-$lang"
        echo "# Empty translation file" > "$REPO_DIR/i18n/Translation-$lang.gz"
    done

    echo "Repository index updated successfully"
    echo ""

    # Create utility scripts
    create_update_repo_script
    create_setup_repo_script
    create_repo_readme

    echo ""
    echo "Local repository setup completed"
    echo ""
}

create_update_repo_script() {
    cat > "$REPO_DIR/update-repo.sh" << 'EOF'
#!/bin/bash
# Script to update local CAOS repository index

REPO_DIR="/opt/caos/repository"
CONF_DIR="$REPO_DIR/conf"
OVERRIDE_FILE="$CONF_DIR/override"

echo "Updating CAOS repository index"

# Create conf directory if it doesn't exist
mkdir -p "$CONF_DIR"

# Create override file if it doesn't exist
if [ ! -f "$OVERRIDE_FILE" ]; then
    cat > "$OVERRIDE_FILE" << OVERRIDE_CONTENT
caos-php optional php
caos-php-8.0 optional php
caos-php-8.1 optional php
caos-php-8.2 optional php
caos-php-8.3 optional php
caos-php-8.4 optional php
OVERRIDE_CONTENT
    echo "Created override file: $OVERRIDE_FILE"
fi

cd "$REPO_DIR" || exit 1

# Remove GPG file
rm -f Release.gpg InRelease

dpkg-scanpackages --multiversion . "$OVERRIDE_FILE" > Packages
gzip -k -f Packages

# Create Release file without GPG references
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
    echo "TIP: Now run: sudo apt update"
    echo "     to refresh APT cache"
else
    echo "ERROR: Failed to update repository index"
    exit 1
fi
EOF
    chmod +x "$REPO_DIR/update-repo.sh"
    echo "Created: update-repo.sh"
}

create_setup_repo_script() {
    cat > "$REPO_DIR/setup-apt-source.sh" << 'EOF'
#!/bin/bash
# Script to configure APT to use local CAOS repository

REPO_DIR="/opt/caos/repository"
APT_SOURCE_FILE="/etc/apt/sources.list.d/caos-local.list"
APT_PREFERENCES_FILE="/etc/apt/preferences.d/caos-local"

if [ "$EUID" -ne 0 ]; then
    echo "ERROR: This script must be run as root (use sudo)"
    exit 1
fi

echo "Setting up CAOS local repository for APT"

# Verify repository exists
if [ ! -d "$REPO_DIR" ]; then
    echo "ERROR: Repository directory not found: $REPO_DIR"
    exit 1
fi

if [ ! -f "$REPO_DIR/Packages.gz" ]; then
    echo "ERROR: Repository index not found. Run: ./update-repo.sh first"
    exit 1
fi

# Create APT source file without GPG verification
echo "deb [trusted=yes] file:$REPO_DIR ./" > "$APT_SOURCE_FILE"

# Create preferences file giving priority to local packages
cat > "$APT_PREFERENCES_FILE" << PREFERENCES
Package: *
Pin: origin ""
Pin-Priority: 1000

Package: caos-php*
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
    echo "TIP: You can now install packages with:"
    echo "     sudo apt install caos-php"
    echo "     sudo apt install caos-php-8.3"
else
    echo "ERROR: Failed to create APT configuration"
    exit 1
fi
EOF
    chmod +x "$REPO_DIR/setup-apt-source.sh"
    echo "Created: setup-apt-source.sh"
}

create_repo_readme() {
    cat > "$REPO_DIR/README.md" << EOF
# CAOS PHP Extension - Local Repository

This directory contains the CAOS PHP extension packages and serves as a local APT repository.

## Available Packages

- caos-php - Meta-package (installs for all detected PHP versions)
- caos-php-8.0 - CAOS extension for PHP 8.0
- caos-php-8.1 - CAOS extension for PHP 8.1
- caos-php-8.2 - CAOS extension for PHP 8.2
- caos-php-8.3 - CAOS extension for PHP 8.3
- caos-php-8.4 - CAOS extension for PHP 8.4

## Initial Setup

### 1. Configure APT to use this repository

sudo ./setup-apt-source.sh

This script will:
- Add the repository to APT sources at /etc/apt/sources.list.d/caos-local.list
- Update the APT cache
- Make packages available for installation via apt

### 2. Install packages

Install meta-package (recommended - installs for all detected PHP versions):
sudo apt install caos-php

Or install specific version:
sudo apt install caos-php-8.3

## Updating the Repository

When you add or update .deb packages in this directory, run:

./update-repo.sh
sudo apt update

The update-repo.sh script regenerates the package index (Packages.gz) that APT uses.

## Manual Installation (without APT)

If you prefer not to use APT:

sudo dpkg -i caos-php-8.3_*.deb
sudo apt-get install -f  # Fix any missing dependencies

## Uninstalling

Remove specific version:
sudo apt remove caos-php-8.3

Remove all versions:
sudo apt remove caos-php-*

## Repository Information

- Repository location: $REPO_DIR
- APT source file: /etc/apt/sources.list.d/caos-local.list
- Repository owner: $USER
- Repository is marked as trusted (no GPG signature required)

## Maintenance

To add new packages:
1. Copy .deb files to this directory
2. Run ./update-repo.sh to regenerate the index
3. Run sudo apt update to refresh APT cache
4. Install or upgrade packages as needed

## Scripts

- setup-apt-source.sh - One-time setup to configure APT (requires sudo)
- update-repo.sh - Regenerate repository index after adding/updating packages (no sudo required)

---

Generated by CAOS build system v$VERSION
EOF
    echo "Created: README.md"
}

verify_packages() {
    echo ""
    echo "Verifying built packages:"
    for pkg in "$BUILD_DIR"/*.deb; do
        if [ -f "$pkg" ]; then
            echo "Package: $(basename "$pkg")"
            dpkg -c "$pkg" | head -5
            echo "   ..."
        fi
    done
}

show_installation_instructions() {
    echo ""
    echo "================================================================"
    echo "All packages built successfully"
    echo "================================================================"
    echo ""
    echo "Local repository location: $REPO_DIR"
    echo ""
    echo "NEXT STEPS:"
    echo ""
    echo "1. Configure APT to use the local repository (one-time setup):"
    echo "   cd $REPO_DIR"
    echo "   sudo ./setup-apt-source.sh"
    echo ""
    echo "2. Install CAOS PHP extension:"
    echo "   sudo apt install caos-php"
    echo ""
    echo "================================================================"
    echo ""
    echo "AVAILABLE SCRIPTS IN REPOSITORY:"
    echo ""
    echo "- setup-apt-source.sh : Configure APT to use this repository"
    echo "                        (requires sudo, run once)"
    echo ""
    echo "- update-repo.sh      : Regenerate repository index after"
    echo "                        adding/updating packages"
    echo "                        (no sudo required)"
    echo ""
    echo "================================================================"
    echo ""
    echo "For detailed instructions, see: $REPO_DIR/README.md"
    echo ""
}

main() {
    # Disable exit-on-error for entire main
    set +e

    # Check repository directory FIRST before doing any work
    check_repository_directory

    check_prerequisites
    if [ $? -ne 0 ]; then
        echo "ERROR: Prerequisites check failed"
        exit 1
    fi

    echo "Building packages for PHP versions: ${PHP_VERSIONS[*]}"
    echo ""

    # Build packages for all versions - continue despite errors
    local built_count=0
    for version in "${PHP_VERSIONS[@]}"; do
        echo "=== PROCESSING PHP $version ==="
        build_single_package "$version"
        if [ $? -eq 0 ]; then
            ((built_count++))
            echo "SUCCESS: PHP $version package built"
        else
            echo "WARNING: PHP $version package skipped (not fatal)"
        fi
        echo "=== COMPLETED PHP $version ==="
        echo ""
    done

    # Create meta-package only if we built at least one package
    if [ $built_count -gt 0 ]; then
        echo "=== CREATING META PACKAGE ==="
        create_meta_package
        echo ""

        verify_packages

        # Setup local repository
        setup_local_repository

        show_installation_instructions

        echo "Multi-version packaging completed"
        echo "Built $built_count packages successfully"
        exit 0
    else
        echo "ERROR: No packages were built successfully"
        exit 1
    fi
}

main "$@"
