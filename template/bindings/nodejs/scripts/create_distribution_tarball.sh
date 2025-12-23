#!/bin/bash

DB_BACKEND=${1:-"MYSQL"}
PROJECT_NAME=${2:-"caos"}
PROJECT_NAME_SANITIZED=$(echo "$PROJECT_NAME" | tr '_' '-')
DB_BACKEND_LOWER=$(echo "$DB_BACKEND" | tr '[:upper:]' '[:lower:]')

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/release"
DIST_DIR="$PROJECT_ROOT/dist"
TEMP_DIR="$DIST_DIR/temp_caos_node_deb"
REPO_DIR="$DIST_DIR/repositories/${PROJECT_NAME}/nodejs"
CAOSDBA_DIR="/opt/caosdba"
TEMP_REPO_DIR="$TEMP_DIR${CAOSDBA_DIR}/repositories/${PROJECT_NAME}/nodejs"
TEMP_DOCS_DIR="$TEMP_DIR${CAOSDBA_DIR}/docs/${PROJECT_NAME}/nodejs"


if [ -f "$BUILD_DIR/build_counter.txt" ]; then
    CAOS_BUILD_COUNTER=$(cat "$BUILD_DIR/build_counter.txt")
    VERSION="1.0.0+${CAOS_BUILD_COUNTER}"
else
    VERSION="1.0.0+1"
    echo "WARNING: $BUILD_DIR/build_counter.txt not found, using default version"
fi

echo "Creating DEB packages tarball for ${PROJECT_NAME} Node.js extension"
echo "Version: $VERSION"
echo "Database backend: $DB_BACKEND_LOWER"

# Function to get Node.js installation paths
get_nodejs_target_dir() {
    if [ -d "/usr/share/nodejs" ]; then
        echo "/usr/share/nodejs"
    elif [ -d "/usr/lib/x86_64-linux-gnu/nodejs" ]; then
        echo "/usr/lib/x86_64-linux-gnu/nodejs"
    else
        echo "/usr/lib/node_modules"
    fi
}

check_prerequisites() {
    if [ ! -d "$REPO_DIR" ]; then
        echo "ERROR: Node.js repository directory not found in $REPO_DIR"
        echo "Please run the package creation first:"
        echo "  cmake --build build/release --target make_package_deb"
        exit 1
    fi

    local deb_count=$(ls "$REPO_DIR"/*.deb 2>/dev/null | wc -l)
    if [ "$deb_count" -eq 0 ]; then
        echo "ERROR: No Node.js DEB packages found in $REPO_DIR"
        echo "Please run the package creation first"
        exit 1
    fi

    echo "Found $deb_count Node.js DEB package(s) in $REPO_DIR"

    # List available packages
    echo "Available packages:"
    for deb_file in "$REPO_DIR"/*.deb; do
        if [ -f "$deb_file" ]; then
            pkg_name=$(basename "$deb_file")
            echo "  - $pkg_name"
        fi
    done
}

create_temporary_structure() {
    rm -rf "$TEMP_DIR"
    mkdir -p "$TEMP_DIR"
    echo "Temporary structure created at $TEMP_DIR"
}

create_readme() {
    local readme_file="$DIST_DIR/README.md"
    local tarball_name="${PROJECT_NAME_SANITIZED}-node-deb-repository-${DB_BACKEND_LOWER}-${VERSION}.tar.gz"

    # Count packages
    local deb_count=$(ls "$REPO_DIR"/*.deb 2>/dev/null | wc -l)

    # Get package list
    local package_list=""
    for deb_file in "$REPO_DIR"/*.deb; do
        if [ -f "$deb_file" ]; then
            pkg_name=$(basename "$deb_file" | cut -d'_' -f1)
            package_list="$package_list\n  - $pkg_name"
        fi
    done

    # Get Node.js target directory
    local TARGET_BASE=$(get_nodejs_target_dir)

    cat > "$readme_file" << EOF
# ${PROJECT_NAME} Node.js DEB Packages Repository

Version: $VERSION
Database Backend: $DB_BACKEND ($DB_BACKEND_LOWER)
Project: $PROJECT_NAME
Total Packages: $deb_count

## Overview

This tarball contains a complete APT repository with ${PROJECT_NAME} Node.js native extensions.
The repository includes packages for multiple Node.js versions detected during build.

## Contents

### Directory Structure

${CAOSDBA_DIR}/
├── install-repository.sh      # Main installation script
├── docs/                      # Documentation
│   └── ${PROJECT_NAME}/       # Project-specific directory
│       └── nodejs/            # Node.js-specific documentation
│           ├── VERSION        # Version information
│           ├── README.md      # This file
│           └── ENV.md         # Environment variables
└── repositories/
    └── ${PROJECT_NAME}/       # Project-specific directory
        └── nodejs/            # Node.js-specific repository
            ├── *.deb          # DEB packages ($deb_count packages)
            ├── Packages       # Repository index
            ├── Packages.gz    # Compressed index
            ├── Release        # Release info
            ├── update-repo.sh # Repository update script
            └── setup-apt-source.sh # APT configuration script

### Installation Location

The Node.js module will be installed to:
$TARGET_BASE/${PROJECT_NAME}/

This directory is automatically detected by Node.js on Ubuntu/Debian systems.

### Available Packages

$package_list

## Installation

### Quick Installation (Recommended)

1. Extract the tarball:
\`\`\`bash
sudo tar -xzf ${tarball_name} -C /
\`\`\`

2. Run the installation script:
\`\`\`bash
sudo ${CAOSDBA_DIR}/install-repository.sh
\`\`\`

### Manual Installation

For advanced users or custom deployments:

1. Extract tarball:
\`\`\`bash
sudo tar -xzf ${tarball_name} -C /
\`\`\`

2. Update repository index:
\`\`\`bash
cd ${CAOSDBA_DIR}/repositories/${PROJECT_NAME}/nodejs
./update-repo.sh
\`\`\`

3. Configure APT source:
\`\`\`bash
./setup-apt-source.sh
\`\`\`

## Usage

### Installing the Extension

\`\`\`bash
# Install meta-package (auto-selects appropriate version)
sudo apt install ${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}

# Install specific version (e.g., for Node.js 22)
sudo apt install ${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}-22

# For exact version (e.g., Node.js 18.19.1)
sudo apt install ${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}-v18_19_1

# List all available packages
apt list ${PROJECT_NAME_SANITIZED}-node*
\`\`\`

### Testing the Installation

\`\`\`bash
# Test with Node.js REPL
node -e "const m = require('${PROJECT_NAME}'); console.log(m.getBuildInfo())"

# Use the global command
${PROJECT_NAME}-node
\`\`\`

### With Multiple Node.js Versions

The extension supports multiple Node.js installations:

\`\`\`bash
# With nvm
nvm use 22
node -e "const m = require('${PROJECT_NAME}'); console.log(m.getBuildInfo())"

nvm use 18
node -e "const m = require('${PROJECT_NAME}'); console.log(m.getBuildInfo())"
\`\`\`

## Multi-Version Support

This repository contains packages built for all Node.js versions found during compilation:

- **System installations**: Packages for Node.js installed via APT
- **User installations**: Packages for Node.js installed via nvm, fnm, or manually
- **Multiple versions**: Different major and minor versions are supported

The extension uses intelligent loading to select the correct native module based on the running Node.js version.

## Maintenance

### Updating the Repository

When new packages are added:

\`\`\`bash
cd ${CAOSDBA_DIR}/repositories/${PROJECT_NAME}/nodejs
./update-repo.sh
sudo apt update
\`\`\`

### Checking Installation

\`\`\`bash
# Check installed packages
dpkg -l | grep ${PROJECT_NAME_SANITIZED}-node

# Check repository status
ls -la ${CAOSDBA_DIR}/repositories/${PROJECT_NAME}/nodejs/*.deb | wc -l
\`\`\`

## Uninstallation

### Remove Repository

1. Remove APT source:
\`\`\`bash
sudo rm -f /etc/apt/sources.list.d/${PROJECT_NAME}-node-local.list
\`\`\`

2. Remove repository files:
\`\`\`bash
sudo rm -rf ${CAOSDBA_DIR}/repositories/${PROJECT_NAME}/nodejs
\`\`\`

3. Update APT cache:
\`\`\`bash
sudo apt update
\`\`\`

### Remove Packages

\`\`\`bash
# Remove all ${PROJECT_NAME} Node.js packages
sudo apt remove ${PROJECT_NAME_SANITIZED}-node-*

# Remove meta-package only (keeps version-specific packages)
sudo apt remove ${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}
\`\`\`

## Technical Details

### Build Information

- **Build System**: CMake with Node.js N-API
- **Node.js Detection**: Automatic detection of system and user installations
- **Package Generation**: One package per detected Node.js version
- **ABI Compatibility**: Uses Node-API for stability across versions

### Environment Variables

See \`${CAOSDBA_DIR}/docs/${PROJECT_NAME}/nodejs/ENV.md\` for complete environment variable documentation.

## Support

For issues or questions:

1. Check the documentation in \`${CAOSDBA_DIR}/docs/${PROJECT_NAME}/nodejs\`
2. Verify Node.js version compatibility
3. Ensure proper repository configuration

---
Generated: $(date)
Build: $VERSION
Database: $DB_BACKEND
Project: $PROJECT_NAME
Language: Node.js
Packages: $deb_count
EOF

    echo "Created README.md in $DIST_DIR"
}

create_env_documentation() {
    local env_file="$DIST_DIR/ENV.md"

    cat > "$env_file" << EOF
# ${PROJECT_NAME} Environment Variables

This document describes all environment variables used by the ${PROJECT_NAME} extension.

## Cache Configuration (Redis)

| Variable Name | Description | Default | Example |
|---------------|-------------|---------|---------|
| \`CAOS_CACHEUSER\` | Username for Redis 6+ with ACL support | - | \`myuser\` |
| \`CAOS_CACHEPASS\` | Authentication password for Redis | - | \`mypassword\` |
| \`CAOS_CACHEHOST\` | Redis server address | \`localhost\` | \`redis.example.com\` |
| \`CAOS_CACHEPORT\` | Redis server port number | \`6379\` | \`6380\` |
| \`CAOS_CACHECLIENTNAME\` | Client name identifier for Redis | - | \`myapp-client\` |
| \`CAOS_CACHEINDEX\` | Redis database number (0-15) | \`0\` | \`1\` |
| \`CAOS_CACHECOMMANDTIMEOUT\` | Timeout for Redis commands (seconds) | \`30\` | \`60\` |
| \`CAOS_CACHEPOOLSIZEMIN\` | Minimum number of connections always kept idle | \`5\` | \`10\` |
| \`CAOS_CACHEPOOLSIZEMAX\` | Maximum number of idle connections in the pool | \`20\` | \`50\` |
| \`CAOS_CACHEPOOLWAIT\` | Timeout when waiting for connection from exhausted pool (seconds) | \`30\` | \`60\` |
| \`CAOS_CACHEPOOLCONNECTIONTIMEOUT\` | Timeout for establishing connection (seconds) | \`10\` | \`30\` |
| \`CAOS_CACHEPOOLCONNECTIONLIFETIME\` | Absolute maximum lifetime of a connection (seconds) | \`3600\` | \`7200\` |
| \`CAOS_CACHEPOOLCONNECTIONIDLETIME\` | Maximum inactivity duration before closing connection (seconds) | \`300\` | \`600\` |

## Database Configuration

| Variable Name | Description | Default | Example |
|---------------|-------------|---------|---------|
| \`CAOS_DBUSER\` | Database username | - | \`dbuser\` |
| \`CAOS_DBPASS\` | Database password | - | \`dbpassword\` |
| \`CAOS_DBHOST\` | Database server address | \`localhost\` | \`db.example.com\` |
| \`CAOS_DBPORT\` | Database server port | \`3306\` | \`5432\` |
| \`CAOS_DBNAME\` | Database name | - | \`myapp_db\` |
| \`CAOS_DBPOOLSIZEMIN\` | Minimum number of database connections in pool | \`5\` | \`10\` |
| \`CAOS_DBPOOLSIZEMAX\` | Maximum number of database connections in pool | \`20\` | \`50\` |
| \`CAOS_DBPOOLWAIT\` | Timeout when waiting for database connection (seconds) | \`30\` | \`60\` |
| \`CAOS_DBPOOLTIMEOUT\` | Database connection pool timeout (seconds) | \`30\` | \`60\` |
| \`CAOS_DBCONNECT_TIMEOUT\` | Database connection timeout (seconds) | \`10\` | \`30\` |
| \`CAOS_DBMAXWAIT\` | Maximum wait time for database operations (seconds) | \`30\` | \`60\` |
| \`CAOS_DBHEALTHCHECKINTERVAL\` | Health check interval for database connections (seconds) | \`30\` | \`60\` |
| \`CAOS_LOG_THRESHOLD_CONNECTION_LIMIT_EXCEEDED\` | Log threshold for connection limit exceeded events | - | \`WARNING\` |
| \`CAOS_VALIDATE_CONNECTION_BEFORE_ACQUIRE\` | Validate connection before acquiring from pool (\`true\`/\`false\`) | \`true\` | \`false\` |
| \`CAOS_VALIDATE_USING_TRANSACTION\` | Validate connection using transaction (\`true\`/\`false\`) | \`false\` | \`true\` |

## Usage Examples

### Basic Configuration

\`\`\`bash
export CAOS_CACHEHOST="..."
export CAOS_CACHEPASS="..."
export CAOS_CACHEPORT=...
export CAOS_DBHOST="..."
export CAOS_DBPORT=...
export CAOS_DBUSER="..."
export CAOS_DBPASS="..."
export CAOS_DBNAME="..."
\`\`\`

### Docker/Container Environment

\`\`\`bash
# Set in Dockerfile
ENV CAOS_CACHEHOST=redis
ENV CAOS_CACHEPORT=6379
ENV CAOS_DBHOST=postgres
ENV CAOS_DBUSER=app
ENV CAOS_DBPASS=password
ENV CAOS_DBNAME=app_db
\`\`\`

## Node.js Integration

The Node.js binding automatically reads these environment variables when loaded.

### Checking Current Configuration

\`\`\`javascript
const caos = require('${PROJECT_NAME}');
console.log('Build Information:', caos.getBuildInfo());
\`\`\`


## Notes

### Security

- Never commit passwords or secrets to version control
- Use environment-specific configuration files
- Consider using secret management systems in production

### Performance

- Adjust connection pool sizes based on your application's concurrency requirements
- Monitor Redis and database connection usage
- Set appropriate timeouts based on network latency

### Compatibility

- The extension supports multiple Node.js versions simultaneously
- Uses Node-API for ABI stability
- Automatic version detection and fallback

### Troubleshooting

1. **Module not found**: Ensure the correct Node.js version package is installed
2. **Connection issues**: Verify environment variables and network connectivity
3. **Performance problems**: Adjust pool sizes and timeouts
4. **Version mismatches**: Check Node.js version compatibility

For more information, refer to the ${PROJECT_NAME} documentation or check the logs with debug mode enabled.
EOF

    echo "Created ENV.md in $DIST_DIR"
}

create_tarball_structure() {
    echo "Creating tarball structure in $TEMP_DIR"

    # Create all necessary directories
    mkdir -p "$TEMP_REPO_DIR"
    mkdir -p "$TEMP_DOCS_DIR"

    if [ -d "$REPO_DIR" ]; then
        echo "Copying Node.js repository from $REPO_DIR to $TEMP_REPO_DIR"
        cp -r "$REPO_DIR"/* "$TEMP_REPO_DIR/"
    else
        echo "ERROR: Source repository $REPO_DIR not found"
        exit 1
    fi

    if [ -f "$DIST_DIR/README.md" ]; then
        echo "Copying README.md from $DIST_DIR to $TEMP_DOCS_DIR"
        cp "$DIST_DIR/README.md" "$TEMP_DOCS_DIR/"
    else
        echo "ERROR: README.md not found in $DIST_DIR"
        echo "This should have been created by create_readme()"
        exit 1
    fi

    if [ -f "$DIST_DIR/ENV.md" ]; then
        echo "Copying ENV.md from $DIST_DIR to $TEMP_DOCS_DIR"
        cp "$DIST_DIR/ENV.md" "$TEMP_DOCS_DIR/"
    else
        echo "ERROR: ENV.md not found in $DIST_DIR"
        echo "This should have been created by create_env_documentation()"
        exit 1
    fi

    # Create version info file in docs directory
    cat > "$TEMP_DOCS_DIR/VERSION" << EOF
PROJECT=${PROJECT_NAME}
VERSION=${VERSION}
DB_BACKEND=${DB_BACKEND}
BUILD_DATE=$(date)
NODE_EXTENSION=true
EOF

    # Set permissions
    echo "Setting permissions..."
    chown -R root:root "$TEMP_DIR" 2>/dev/null || true
    chmod 755 "$TEMP_DIR${CAOSDBA_DIR}" 2>/dev/null || true
    find "$TEMP_DIR" -type d -exec chmod 755 {} \;
    find "$TEMP_DIR" -type f -exec chmod 644 {} \;

    chmod 755 "$TEMP_DIR${CAOSDBA_DIR}/install-repository.sh" 2>/dev/null || true
    chmod 755 "$TEMP_REPO_DIR/update-repo.sh" 2>/dev/null || true
    chmod 755 "$TEMP_REPO_DIR/setup-apt-source.sh" 2>/dev/null || true
    chmod 640 "$TEMP_DOCS_DIR/VERSION" 2>/dev/null || true

    echo "Tarball structure created successfully"
    echo "Documentation location: ${CAOSDBA_DIR}/docs/${PROJECT_NAME}/nodejs/"
}

create_install_script() {
    mkdir -p "$TEMP_DIR${CAOSDBA_DIR}"

    cat > "$TEMP_DIR${CAOSDBA_DIR}/install-repository.sh" << 'EOF'
#!/bin/bash
# __PROJECT_NAME__ Node.js Repository Installation Script

set -e

CAOSDBA_DIR="/opt/caosdba"
REPO_DIR="$CAOSDBA_DIR/repositories/__PROJECT_NAME__/nodejs"
VERSION_FILE="$CAOSDBA_DIR/docs/__PROJECT_NAME__/nodejs/VERSION"

if [ "$EUID" -ne 0 ]; then
    echo "ERROR: This script must be run as root (use sudo)"
    exit 1
fi

echo "================================================================"
echo "__PROJECT_NAME__ Node.js Repository Installation"
echo "================================================================"

if [ ! -d "$REPO_DIR" ]; then
    echo "ERROR: Node.js repository directory not found: $REPO_DIR"
    echo "Make sure the tarball was extracted to /opt/caosdba"
    exit 1
fi

# Display version information
if [ -f "$VERSION_FILE" ]; then
    echo "Version Information:"
    echo "-------------------"
    cat "$VERSION_FILE"
    echo ""
fi

echo "Setting up __PROJECT_NAME__ Node.js repository from $REPO_DIR"

# Clean up any existing conflicting source files
echo "Cleaning up existing repository configurations..."
for file in /etc/apt/sources.list.d/*__PROJECT_NAME__*.list; do
    if [ -f "$file" ]; then
        echo "  Removing: $(basename "$file")"
        rm -f "$file"
    fi
done

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
echo "__PROJECT_NAME__ Node.js Repository Installation Completed"
echo "================================================================"
echo ""
echo "Summary:"
echo "  Project: __PROJECT_NAME__"
echo "  Version: __VERSION__"
echo "  Database: __DB_BACKEND__ (__DB_BACKEND_LOWER__)"
echo "  Packages: $(ls "$REPO_DIR"/*.deb 2>/dev/null | wc -l) DEB packages"
echo ""
echo "Installation options:"
echo "  1. Install meta-package (recommended):"
echo "     sudo apt install __PROJECT_NAME_SANITIZED__-node-__DB_BACKEND_LOWER__"
echo ""
echo "  2. Install specific Node.js version:"
echo "     sudo apt install __PROJECT_NAME_SANITIZED__-node-__DB_BACKEND_LOWER__-<version>"
echo ""
echo "  3. List all available packages:"
echo "     apt list __PROJECT_NAME_SANITIZED__-node*"
echo ""
echo "Testing:"
echo "  node -e \"const m = require('__PROJECT_NAME__'); console.log(m.getBuildInfo())\""
echo ""
echo "Or use the global command:"
echo "  __PROJECT_NAME__-node"
echo ""
echo "Documentation: $CAOSDBA_DIR/docs/__PROJECT_NAME__/nodejs"
echo "Repository location: $REPO_DIR"
echo ""
EOF

    sed -i "s/__PROJECT_NAME__/${PROJECT_NAME}/g" "$TEMP_DIR${CAOSDBA_DIR}/install-repository.sh"
    sed -i "s/__PROJECT_NAME_SANITIZED__/${PROJECT_NAME_SANITIZED}/g" "$TEMP_DIR${CAOSDBA_DIR}/install-repository.sh"
    sed -i "s/__VERSION__/${VERSION}/g" "$TEMP_DIR${CAOSDBA_DIR}/install-repository.sh"
    sed -i "s/__DB_BACKEND__/${DB_BACKEND}/g" "$TEMP_DIR${CAOSDBA_DIR}/install-repository.sh"
    sed -i "s/__DB_BACKEND_LOWER__/${DB_BACKEND_LOWER}/g" "$TEMP_DIR${CAOSDBA_DIR}/install-repository.sh"

    chmod 755 "$TEMP_DIR${CAOSDBA_DIR}/install-repository.sh"
    echo "Created install-repository.sh in tarball structure"
}

create_tarball() {
    local tarball_name="${PROJECT_NAME_SANITIZED}-node-deb-repository-${DB_BACKEND_LOWER}-${VERSION}.tar.gz"
    local tarball_path="$DIST_DIR/$tarball_name"

    echo "Creating Node.js tarball: $tarball_name"

    mkdir -p "$DIST_DIR"
    tar -czf "$tarball_path" -C "$TEMP_DIR" opt

    local deb_count=$(ls "$TEMP_REPO_DIR"/*.deb 2>/dev/null | wc -l)

    rm -rf "$TEMP_DIR"

    echo "Node.js tarball created: $tarball_path"
    echo "Contains $deb_count Node.js DEB package(s)"
    echo ""
    echo "To deploy:"
    echo "  sudo tar -xzf $tarball_path -C /"
    echo "  sudo ${CAOSDBA_DIR}/install-repository.sh"
    echo ""
}

create_deployment_readme() {
    local tarball_name="${PROJECT_NAME_SANITIZED}-node-deb-repository-${DB_BACKEND_LOWER}-${VERSION}.tar.gz"
    local readme_file="$DIST_DIR/${tarball_name}.README.txt"

    cat > "$readme_file" << EOF
${PROJECT_NAME} Node.js DEB Repository
======================================

Tarball: ${tarball_name}
Version: ${VERSION}
Database: ${DB_BACKEND} (${DB_BACKEND_LOWER})

Deployment Instructions:
=======================

1. Extract the tarball:
   sudo tar -xzf ${tarball_name} -C /

2. Run the installation script:
   sudo ${CAOSDBA_DIR}/install-repository.sh

3. Install the extension:
   # Install meta-package (recommended):
   sudo apt install ${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}

   # Or install specific version:
   sudo apt install ${PROJECT_NAME_SANITIZED}-node-${DB_BACKEND_LOWER}-<version>

4. Test the installation:
   node -e "const m = require('${PROJECT_NAME}'); console.log(m.getBuildInfo())"

   # Or use the global command:
   ${PROJECT_NAME}-node

Additional Information:
=======================

- Documentation: ${CAOSDBA_DIR}/docs/${PROJECT_NAME}/nodejs/
- Repository: ${CAOSDBA_DIR}/repositories/${PROJECT_NAME}/nodejs/
- Version info: ${CAOSDBA_DIR}/docs/${PROJECT_NAME}/nodejs/VERSION

Support for multiple Node.js versions:
- System installations (via APT)
- User installations (via nvm, fnm)
- Automatic version detection
- Intelligent fallback mechanism

For issues or questions, check the documentation in ${CAOSDBA_DIR}/docs/${PROJECT_NAME}/nodejs/
EOF

    echo "Created deployment README: ${tarball_name}.README.txt"
}

main() {
    echo "Starting Node.js DEB repository tarball creation for $PROJECT_NAME..."

    check_prerequisites
    create_temporary_structure
    create_readme
    create_env_documentation
    create_install_script
    create_tarball_structure
    create_tarball
    create_deployment_readme
}

main "$@"
