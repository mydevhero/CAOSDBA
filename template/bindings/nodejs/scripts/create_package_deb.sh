#!/bin/bash

DB_BACKEND=${1:-"MYSQL"}
PROJECT_NAME=${2:-"caos"}
PROJECT_NAME_SANITIZED=$(echo "$PROJECT_NAME" | tr '_' '-')
DB_BACKEND_LOWER=$(echo "$DB_BACKEND" | tr '[:upper:]' '[:lower:]')

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/release"
DIST_DIR="$PROJECT_ROOT/dist"
REPO_DIR="$DIST_DIR/repositories/${PROJECT_NAME}/nodejs"

if [ -f "$BUILD_DIR/build_counter.txt" ]; then
    CAOS_BUILD_COUNTER=$(cat "$BUILD_DIR/build_counter.txt")
    VERSION="1.0.0+${CAOS_BUILD_COUNTER}"
else
    VERSION="1.0.0+1"
    echo "WARNING: Using default version"
fi

# Function to get Node.js installation paths on Ubuntu/Debian
get_nodejs_paths() {
    # Standard paths where Node.js looks for modules on Ubuntu/Debian
    echo "/usr/lib/x86_64-linux-gnu/nodejs"
    echo "/usr/share/nodejs"
    echo "/usr/lib/node_modules"
    echo "/usr/local/lib/node_modules"
}

# Function to determine the correct target directory for this system
get_nodejs_target_dir() {
    # First check standard paths that exist on the system
    for path in $(get_nodejs_paths); do
        if [ -d "$path" ]; then
            echo "$path"
            return 0
        fi
    done

    # Fallback to standard location
    echo "/usr/lib/node_modules"
}

extract_node_version() {
    local dirname=$1
    local basename=$(basename "$dirname")

    # Pattern: v14, v18.19.1, v22.10.0
    if [[ "$basename" =~ ^v([0-9]+) ]]; then
        local major_version="${BASH_REMATCH[1]}"

        # Check if it's a full version (e.g., v18.19.1)
        if [[ "$basename" =~ ^v([0-9]+)\.([0-9]+)\.([0-9]+)$ ]]; then
            echo "${major_version}|${basename}"
        else
            echo "${major_version}|${basename}"
        fi
    else
        echo ""
    fi
}

extract_build_number() {
    if [ -f "$BUILD_DIR/build_counter.txt" ]; then
        cat "$BUILD_DIR/build_counter.txt"
    else
        echo "1"
    fi
}

find_node_extension_files() {
    # Look in both dist directory and build directory
    local files=""

    # Check dist directory (from CMake copy)
    if [ -d "$REPO_DIR" ]; then
        files=$(find "$REPO_DIR" -name "${PROJECT_NAME}.node" -type f 2>/dev/null)
    fi

    # Also check build directory directly
    if [ -z "$files" ] && [ -d "$BUILD_DIR" ]; then
        files=$(find "$BUILD_DIR" -name "${PROJECT_NAME}.node" -type f 2>/dev/null)
    fi

    echo "$files"
}

calculate_node_paths() {
    local node_version=$1
    # Use system-specific target directory
    local TARGET_BASE=$(get_nodejs_target_dir)
    NODE_MODULE_DIR="$TARGET_BASE/${PROJECT_NAME}"

    echo "Using Node.js target directory: $TARGET_BASE"
}

check_prerequisites() {
    # Check if any Node.js extensions were built
    local ext_files=$(find_node_extension_files)

    if [ -z "$ext_files" ]; then
        echo "ERROR: No Node.js extension files found"
        echo "Expected pattern: ${PROJECT_NAME}.node"
        echo "Please build the project first:"
        echo "  cmake --build build/release --target ${PROJECT_NAME}_node_all"
        exit 1
    fi

    local file_count=$(echo "$ext_files" | wc -l)

    if [ "$file_count" -eq 0 ]; then
        echo "ERROR: No Node.js extension files found"
        exit 1
    fi

    if ! command -v dpkg-deb >/dev/null 2>&1; then
        echo "ERROR: dpkg-deb not found. Install dpkg-dev"
        exit 1
    fi

    echo "Found $file_count Node.js extension file(s)"
}

create_structure() {
    local deb_dir=$1
    local node_version=$2
    local node_dir_name=$3

    # Get the correct target base directory
    local TARGET_BASE=$(get_nodejs_target_dir)
    local MODULE_DIR="$TARGET_BASE/${PROJECT_NAME}"

    rm -rf "$deb_dir"
    mkdir -p "$deb_dir/$MODULE_DIR/$node_dir_name"
    mkdir -p "$deb_dir/DEBIAN"
}

copy_files() {
    local deb_dir=$1
    local source_file=$2
    local node_version=$3
    local node_dir_name=$4
    local source_dir=$(dirname "$source_file")

    # Get the correct target base directory
    local TARGET_BASE=$(get_nodejs_target_dir)
    local MODULE_DIR="$TARGET_BASE/${PROJECT_NAME}"
    local VERSION_DIR="$deb_dir/$MODULE_DIR/$node_dir_name"

    # Create version directory
    mkdir -p "$VERSION_DIR"

    # Copy .node file to version directory
    cp "$source_file" "$VERSION_DIR/"

    # Copy index.js and package.json if they exist
    if [ -f "$source_dir/index.js" ]; then
        cp "$source_dir/index.js" "$VERSION_DIR/"
    fi

    if [ -f "$source_dir/package.json" ]; then
        cp "$source_dir/package.json" "$VERSION_DIR/"
    fi

    # Create main launcher.js in the module directory
    if [ ! -f "$deb_dir/$MODULE_DIR/launcher.js" ]; then
        cat > "$deb_dir/$MODULE_DIR/launcher.js" << 'EOF'
#!/usr/bin/env node
const path = require('path');
const fs = require('fs');
const PROJECT_NAME = '__PROJECT_NAME__';
function loadNativeModule() {
    const moduleDir = __dirname;
    const nodeVersion = process.versions.node;
    const nodeMajor = nodeVersion.split('.')[0];
    const exactVersion = `v${nodeVersion}`;
    const exactModule = path.join(moduleDir, exactVersion, `${PROJECT_NAME}.node`);
    if (fs.existsSync(exactModule)) {
        return require(exactModule);
    }
    const majorModule = path.join(moduleDir, `v${nodeMajor}`, `${PROJECT_NAME}.node`);
    if (fs.existsSync(majorModule)) {
        return require(majorModule);
    }
    const files = fs.readdirSync(moduleDir);
    let availableVersions = [];
    for (const file of files) {
        if (file.startsWith('v') && fs.existsSync(path.join(moduleDir, file, `${PROJECT_NAME}.node`))) {
            availableVersions.push(file);
        }
    }
    if (availableVersions.length > 0) {
        availableVersions.sort((a, b) => {
            const aVer = parseInt(a.replace(/^v/, '').split('.')[0]);
            const bVer = parseInt(b.replace(/^v/, '').split('.')[0]);
            return Math.abs(aVer - nodeMajor) - Math.abs(bVer - nodeMajor);
        });
        const closestVersion = availableVersions[0];
        console.warn(`[${PROJECT_NAME}] Using Node.js ${closestVersion} module for Node.js ${nodeVersion}`);
        return require(path.join(moduleDir, closestVersion, `${PROJECT_NAME}.node`));
    }
    throw new Error(`No native module found for Node.js ${nodeVersion} in ${moduleDir}`);
}
const native = loadNativeModule();
if (require.main === module) {
    if (native.getBuildInfo) {
        console.log(JSON.stringify(native.getBuildInfo(), null, 2));
    } else {
        console.log(`{ "module": "${PROJECT_NAME}", "status": "loaded" }`);
    }
}
module.exports = native;
EOF
        sed -i "s/__PROJECT_NAME__/${PROJECT_NAME}/g" "$deb_dir/$MODULE_DIR/launcher.js"
    fi

    # Create main package.json in the module directory (REQUIRED FOR NODE.JS)
    if [ ! -f "$deb_dir/$MODULE_DIR/package.json" ]; then
        cat > "$deb_dir/$MODULE_DIR/package.json" << EOF
{
  "name": "${PROJECT_NAME}",
  "version": "0.1.0",
  "main": "./launcher.js",
  "description": "CAOS Native Node.js Bindings for ${DB_BACKEND}",
  "engines": {
    "node": ">=${node_version}.0.0"
  }
}
EOF
    fi

    # Create index.js symlink to launcher.js (REQUIRED FOR NODE.JS)
    if [ ! -f "$deb_dir/$MODULE_DIR/index.js" ]; then
        # Create a symlink in the DEB package
        ln -s "launcher.js" "$deb_dir/$MODULE_DIR/index.js"
    fi
}

create_postinst() {
    local deb_dir=$1
    local node_version=$2
    local node_dir_name=$3

    # Get the correct target base directory
    local TARGET_BASE=$(get_nodejs_target_dir)
    local MODULE_DIR="$TARGET_BASE/${PROJECT_NAME}"

    cat > "$deb_dir/DEBIAN/postinst" << EOF
#!/bin/bash
set -e

PACKAGE_NAME="${PROJECT_NAME}"
DB_BACKEND="${DB_BACKEND}"
DB_BACKEND_LOWER="${DB_BACKEND_LOWER}"
NODE_MODULE_DIR="$MODULE_DIR"

echo "Configuring \${PACKAGE_NAME} Node.js module..."

if [ -d "\$NODE_MODULE_DIR" ]; then
    echo "  Setting up Node.js module in \$NODE_MODULE_DIR"

    # Fix permissions for all version directories
    find "\$NODE_MODULE_DIR" -type d -name "v*" -exec chmod 755 {} \; 2>/dev/null || true
    find "\$NODE_MODULE_DIR" -type f -name "*.node" -exec chmod 644 {} \; 2>/dev/null || true
    find "\$NODE_MODULE_DIR" -type f -name "*.js" -exec chmod 644 {} \; 2>/dev/null || true
    find "\$NODE_MODULE_DIR" -type f -name "*.json" -exec chmod 644 {} \; 2>/dev/null || true

    if [ -f "\$NODE_MODULE_DIR/launcher.js" ]; then
        chmod 754 "\$NODE_MODULE_DIR/launcher.js"
        echo "  Set executable permissions on launcher.js"
    else
        echo "  WARNING: launcher.js not found in \$NODE_MODULE_DIR"
    fi

    # Create index.js symlink if it doesn't exist (REQUIRED FOR NODE.JS REQUIRE)
    if [ -f "\$NODE_MODULE_DIR/launcher.js" ] && [ ! -f "\$NODE_MODULE_DIR/index.js" ]; then
        ln -sf "launcher.js" "\$NODE_MODULE_DIR/index.js"
        echo "  Created index.js symlink for Node.js compatibility"
    fi

    # Ensure package.json exists in main directory (REQUIRED FOR NODE.JS REQUIRE)
    if [ ! -f "\$NODE_MODULE_DIR/package.json" ]; then
        cat > "\$NODE_MODULE_DIR/package.json" << PKG_EOF
{
  "name": "${PROJECT_NAME}",
  "version": "0.1.0",
  "main": "./launcher.js",
  "description": "CAOS Native Node.js Bindings for ${DB_BACKEND}",
  "engines": {
    "node": ">=${node_version}.0.0"
  }
}
PKG_EOF
        echo "  Created main package.json for Node.js compatibility"
    fi

    # Create global symlink for CLI access
    if [ -f "\$NODE_MODULE_DIR/launcher.js" ]; then
        ln -sf "\$NODE_MODULE_DIR/launcher.js" "/usr/local/bin/\${PACKAGE_NAME}-node" 2>/dev/null || true
        echo "  Created global command: \${PACKAGE_NAME}-node"
    fi

    echo "  Installed Node.js module for version: ${node_dir_name}"
else
    echo "  WARNING: Node.js module directory not found: \$NODE_MODULE_DIR"
fi

echo "Node.js module installation completed."
EOF

    chmod +x "$deb_dir/DEBIAN/postinst"
}

create_prerm() {
    local deb_dir=$1
    local node_version=$2
    local node_dir_name=$3

    # Get the correct target base directory
    local TARGET_BASE=$(get_nodejs_target_dir)
    local MODULE_DIR="$TARGET_BASE/${PROJECT_NAME}"

    cat > "$deb_dir/DEBIAN/prerm" << EOF
#!/bin/bash
set -e

PACKAGE_NAME="${PROJECT_NAME}"
NODE_MODULE_DIR="$MODULE_DIR"

# Only remove the global symlink if no other versions are installed
if [ -d "\$NODE_MODULE_DIR" ]; then
    # Count remaining .node files
    NODE_FILE_COUNT=\$(find "\$NODE_MODULE_DIR" -name "*.node" -type f 2>/dev/null | wc -l)

    if [ "\$NODE_FILE_COUNT" -le 1 ]; then
        # This is the last version, remove symlink
        rm -f "/usr/local/bin/\${PACKAGE_NAME}-node" 2>/dev/null || true
    fi
fi
EOF

    chmod +x "$deb_dir/DEBIAN/prerm"
}

create_control() {
    local deb_dir=$1
    local package_name=$2
    local node_version=$3
    local node_dir_name=$4
    local architecture="amd64"

    # Create dependency based on Node.js version
    local node_dep=""

    # Check if this is a system package version (e.g., v18 vs v18.19.1)
    if [[ "$node_dir_name" =~ ^v[0-9]+$ ]]; then
        # Major version only (e.g., v18) - depends on system nodejs AND meta-package
        node_dep="nodejs (>= ${node_version}.0.0), ${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}"
    else
        # Full version (e.g., v18.19.1) - depends on nodejs or any compatible version AND meta-package
        node_dep="nodejs (>= ${node_version}.0.0) | nodejs-${node_version}, ${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}"
    fi

    local conflicts_clause=""
    case "${DB_BACKEND_LOWER}" in
        "mysql")
            conflicts_clause="${PROJECT_NAME_SANITIZED}-node-mariadb-${node_version}, ${PROJECT_NAME_SANITIZED}-node-postgresql-${node_version}"
            ;;
        "mariadb")
            conflicts_clause="${PROJECT_NAME_SANITIZED}-node-mysql-${node_version}, ${PROJECT_NAME_SANITIZED}-node-postgresql-${node_version}"
            ;;
        "postgresql")
            conflicts_clause="${PROJECT_NAME_SANITIZED}-node-mysql-${node_version}, ${PROJECT_NAME_SANITIZED}-node-mariadb-${node_version}"
            ;;
        *)
            ;;
    esac

    cat > "$deb_dir/DEBIAN/control" << EOF
Package: $package_name
Version: $VERSION
Architecture: $architecture
Maintainer: CAOSDBA Development Team
Description: ${PROJECT_NAME} - CAOSDBA extension for Node.js ${node_dir_name} with $DB_BACKEND_LOWER backend
 Native Node.js bindings for CAOSDBA database operations.
 Built for Node.js version: ${node_dir_name}
Depends: $node_dep
Conflicts: $conflicts_clause
Section: javascript
Priority: optional
EOF
}

build_single_package() {
    local source_file=$1
    local node_version=$2
    local node_dir_name=$3
    local build_number=$4

    calculate_node_paths "$node_version"

    # Create package name based on version
    local package_name=""
    if [[ "$node_dir_name" =~ ^v[0-9]+$ ]]; then
        # Major version only
        package_name="${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}-${node_version}"
    else
        # Full version - sanitize for package name
        local safe_dir_name=$(echo "$node_dir_name" | tr '.' '_' | tr '-' '_')
        package_name="${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}-${safe_dir_name}"
    fi

    local deb_dir="$BUILD_DIR/deb-node-${node_dir_name}"

    create_structure "$deb_dir" "$node_version" "$node_dir_name"
    copy_files "$deb_dir" "$source_file" "$node_version" "$node_dir_name"

    create_postinst "$deb_dir" "$node_version" "$node_dir_name"
    create_prerm "$deb_dir" "$node_version" "$node_dir_name"
    create_control "$deb_dir" "$package_name" "$node_version" "$node_dir_name"

    dpkg-deb --build "$deb_dir" "$BUILD_DIR/${package_name}_${VERSION}_amd64.deb" >/dev/null 2>&1

    rm -rf "$deb_dir"

    echo "Built: ${package_name}_${VERSION}_amd64.deb (Node.js ${node_dir_name})"
    return 0
}

create_meta_package() {
    local deb_dir="$BUILD_DIR/deb-meta-node"
    mkdir -p "$deb_dir/DEBIAN"

    # Collect all built packages
    local recommends_clause=""
    local package_list=""

    for pkg_file in "$BUILD_DIR"/*-node-${DB_BACKEND_LOWER}-*_${VERSION}_amd64.deb; do
        if [ -f "$pkg_file" ]; then
            pkg_name=$(basename "$pkg_file" | cut -d'_' -f1)
            recommends_clause="$recommends_clause$pkg_name (>= $VERSION), "

            # Extract version info for description
            if [[ "$pkg_name" =~ ${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}-([0-9_]+)$ ]]; then
                version_part="${BASH_REMATCH[1]}"
                package_list="$package_list$version_part, "
            fi
        fi
    done

    if [ -n "$recommends_clause" ]; then
        recommends_clause="${recommends_clause%, }"
    else
        recommends_clause="dummy-package"
        echo "WARNING: No version-specific packages found for meta-package"
    fi

    package_list="${package_list%, }"

    local meta_conflicts_clause=""
    case "${DB_BACKEND_LOWER}" in
        "mysql")
            meta_conflicts_clause="${PROJECT_NAME_SANITIZED}-node-mariadb, ${PROJECT_NAME_SANITIZED}-node-postgresql"
            ;;
        "mariadb")
            meta_conflicts_clause="${PROJECT_NAME_SANITIZED}-node-mysql, ${PROJECT_NAME_SANITIZED}-node-postgresql"
            ;;
        "postgresql")
            meta_conflicts_clause="${PROJECT_NAME_SANITIZED}-node-mysql, ${PROJECT_NAME_SANITIZED}-node-mariadb"
            ;;
        *)
            ;;
    esac

    cat > "$deb_dir/DEBIAN/control" << EOF
Package: ${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}
Version: $VERSION
Architecture: all
Maintainer: CAOSDBA Development Team
Description: ${PROJECT_NAME} - CAOSDBA Node.js extension with $DB_BACKEND_LOWER backend (metapackage)
 This metapackage will install the appropriate Node.js version package.
 Includes support for: ${package_list}
Recommends: $recommends_clause
Conflicts: $meta_conflicts_clause
Section: javascript
Priority: optional
EOF

    dpkg-deb --build "$deb_dir" "$BUILD_DIR/${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}_${VERSION}_all.deb" >/dev/null 2>&1

    rm -rf "$deb_dir"
    echo "Built meta-package: ${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}_${VERSION}_all.deb"
}

create_repository_scripts() {
    mkdir -p "$REPO_DIR"

    # update-repo.sh - usa EOF con sostituzione di variabili
    cat > "$REPO_DIR/update-repo.sh" << EOF
#!/bin/bash

REPO_DIR="\$(cd "\$(dirname "\$0")" && pwd)"
CONF_DIR="\$REPO_DIR/conf"
OVERRIDE_FILE="\$CONF_DIR/override"

mkdir -p "\$CONF_DIR"

> "\$OVERRIDE_FILE"
for deb_file in "\$REPO_DIR"/*.deb; do
    if [ -f "\$deb_file" ]; then
        package_name=\$(dpkg-deb -f "\$deb_file" Package 2>/dev/null || basename "\$deb_file" | cut -d'_' -f1)
        if [ -n "\$package_name" ]; then
            echo "\$package_name optional nodejs" >> "\$OVERRIDE_FILE"
        fi
    fi
done

cd "\$REPO_DIR" || exit 1

rm -f Release.gpg InRelease

dpkg-scanpackages --multiversion . "\$OVERRIDE_FILE" > Packages
gzip -k -f Packages

cat > Release << RELEASE_CONTENT
Origin: ${PROJECT_NAME} Node.js Repository
Label: ${PROJECT_NAME}
Suite: stable
Codename: ${PROJECT_NAME}
Version: 1.0
Architectures: amd64 all
Components: main
Description: ${PROJECT_NAME} - CAOSDBA Node.js extension
Date: \$(date -Ru)
MD5Sum:
 \$(md5sum Packages | cut -d' ' -f1) \$(stat -c %s Packages) Packages
 \$(md5sum Packages.gz | cut -d' ' -f1) \$(stat -c %s Packages.gz) Packages.gz
SHA256:
 \$(sha256sum Packages | cut -d' ' -f1) \$(stat -c %s Packages) Packages
 \$(sha256sum Packages.gz | cut -d' ' -f1) \$(stat -c %s Packages.gz) Packages.gz
RELEASE_CONTENT

echo "Node.js repository index updated"
echo "Available packages:"
for deb_file in *.deb; do
    if [ -f "\$deb_file" ]; then
        echo "  - \$(basename "\$deb_file")"
    fi
done
EOF
    chmod +x "$REPO_DIR/update-repo.sh"

    # setup-apt-source.sh
    cat > "$REPO_DIR/setup-apt-source.sh" << EOF
#!/bin/bash

REPO_DIR="\$(cd "\$(dirname "\$0")" && pwd)"
APT_SOURCE_FILE="/etc/apt/sources.list.d/${PROJECT_NAME}-node-local.list"

if [ "\$EUID" -ne 0 ]; then
    echo "ERROR: This script must be run as root"
    exit 1
fi

if [ ! -d "\$REPO_DIR" ] || [ ! -f "\$REPO_DIR/Packages.gz" ]; then
    echo "ERROR: Repository not found or not indexed"
    echo "Please run ./update-repo.sh first"
    exit 1
fi

echo "Cleaning up existing repository configurations..."
for file in /etc/apt/sources.list.d/${PROJECT_NAME}-node-local.list; do
    if [ -f "\$file" ]; then
        echo "  Removing: \$(basename "\$file")"
        rm -f "\$file"
    fi
done

echo "deb [trusted=yes] file:\$REPO_DIR ./" > "\$APT_SOURCE_FILE"

# Forziamo l'aggiornamento senza chiedere conferma
apt-get update -oAcquire::AllowInsecureRepositories=true -oAcquire::AllowDowngradeToInsecureRepositories=true

echo ""
echo "=== ${PROJECT_NAME} Node.js Repository Setup Complete ==="
echo ""
echo "Available packages:"
echo "-------------------"
for deb_file in "\$REPO_DIR"/*.deb; do
    if [ -f "\$deb_file" ]; then
        pkg_name=\$(basename "\$deb_file" | cut -d'_' -f1)
        echo "  \$pkg_name"
    fi
done
echo ""
echo "Install meta-package (recommended):"
echo "  sudo apt install ${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}"
echo ""
echo "Or install specific version:"
echo "  sudo apt install ${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}-<version>"
echo ""
echo "Test installation:"
echo "  node -e \"const m = require('${PROJECT_NAME}'); console.log(m.getBuildInfo())\""
EOF
    chmod +x "$REPO_DIR/setup-apt-source.sh"
}

move_packages_to_repository() {
    mkdir -p "$REPO_DIR"

    # Move all .deb files to repository
    local moved_count=0
    for pkg_file in "$BUILD_DIR"/*.deb; do
        if [ -f "$pkg_file" ]; then
            mv "$pkg_file" "$REPO_DIR/"
            moved_count=$((moved_count + 1))
        fi
    done

    echo "Moved $moved_count package(s) to repository: $REPO_DIR"
}

main() {
    check_prerequisites

    local ext_files=$(find_node_extension_files)
    local built_count=0

    # Read installation mappings if available
    local installation_map=""
    if [ -f "$BUILD_DIR/node_installations.txt" ]; then
        installation_map=$(cat "$BUILD_DIR/node_installations.txt")
    fi

    while IFS= read -r source_file; do
        if [ -z "$source_file" ]; then
            continue
        fi

        # Extract version from directory structure
        local source_dir=$(dirname "$source_file")
        local dir_name=$(basename "$source_dir")

        # Try to extract version from directory name
        local version_info=$(extract_node_version "$dir_name")

        if [ -z "$version_info" ]; then
            # Try parent directory
            local parent_dir=$(dirname "$source_dir")
            local parent_name=$(basename "$parent_dir")
            version_info=$(extract_node_version "$parent_name")
        fi

        if [ -z "$version_info" ]; then
            echo "WARNING: Cannot extract Node.js version from path: $source_file"
            continue
        fi

        # Parse version info (format: major|dir_name)
        IFS='|' read -r node_major node_dir_name <<< "$version_info"

        if [ -z "$node_major" ] || [ -z "$node_dir_name" ]; then
            echo "WARNING: Invalid version format: $version_info"
            continue
        fi

        local build_number=$(extract_build_number "$source_file")

        if build_single_package "$source_file" "$node_major" "$node_dir_name" "$build_number"; then
            built_count=$((built_count + 1))
        fi
    done <<< "$ext_files"

    if [ "$built_count" -gt 0 ]; then
        create_meta_package
        move_packages_to_repository
        create_repository_scripts

        # Create summary file
        local summary_file="$REPO_DIR/PACKAGES_SUMMARY.txt"
        cat > "$summary_file" << EOF
${PROJECT_NAME} Node.js DEB Packages Summary
============================================

Project: ${PROJECT_NAME}
Database Backend: ${DB_BACKEND} (${DB_BACKEND_LOWER})
Version: ${VERSION}
Build Date: $(date)

Packages Built: ${built_count}

Available Packages:
$(for pkg_file in "$REPO_DIR"/*.deb; do
    if [ -f "$pkg_file" ]; then
        pkg_name=$(basename "$pkg_file")
        echo "  - $pkg_name"
    fi
done)

Installation:
1. Update repository: ./update-repo.sh
2. Configure APT: sudo ./setup-apt-source.sh
3. Install: sudo apt install ${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}

Node.js Versions Supported:
$(echo "$ext_files" | while read -r file; do
    dir=$(dirname "$file")
    version=$(basename "$dir")
    echo "  - $version"
done | sort -u)

Notes:
- The meta-package will install the appropriate version based on your Node.js installation
- Multiple Node.js versions can be installed simultaneously
- Each version is installed in its own directory under the correct Node.js module path

EOF

        echo "Packaging completed: $built_count Node.js packages built"
        echo "Repository location: $REPO_DIR"
        exit 0
    else
        echo "ERROR: No Node.js packages built"
        exit 1
    fi
}

main "$@"
