#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build/release"
DIST_DIR="$PROJECT_ROOT/dist"
CAOS_OPT_DIR="/opt/caosdba"

# Get database backend and project name from command line
DB_BACKEND=${1:-"MYSQL"}
PROJECT_NAME=${2:-"caos"}  # Default to 'caos' for backward compatibility
PROJECT_NAME_SANITIZED=$(echo "$PROJECT_NAME" | tr '_' '-')
DB_BACKEND_LOWER=$(echo "$DB_BACKEND" | tr '[:upper:]' '[:lower:]')

if [ -f "$BUILD_DIR/build_counter.txt" ]; then
    CAOS_BUILD_COUNTER=$(cat "$BUILD_DIR/build_counter.txt")
    VERSION="1.0.0+${CAOS_BUILD_COUNTER}"
else
    VERSION="1.0.0+1"
    echo "WARNING: $BUILD_DIR/build_counter.txt not found, using default version"
fi

echo "Creating DEB packages tarball for ${PROJECT_NAME}"
echo "Version: $VERSION"
echo "Database backend: $DB_BACKEND_LOWER"

check_prerequisites() {
    # Check if repository was created in dist directory
    if [ ! -d "$DIST_DIR/repository" ]; then
        echo "ERROR: Repository directory not found in $DIST_DIR/repository"
        echo "Please run the ${PROJECT_NAME}_package_deb target first:"
        echo "  cmake --build build/release --target ${PROJECT_NAME}_package_deb"
        exit 1
    fi

    local deb_count=$(ls "$DIST_DIR/repository"/*.deb 2>/dev/null | wc -l)
    if [ "$deb_count" -eq 0 ]; then
        echo "ERROR: No DEB packages found in $DIST_DIR/repository"
        echo "Please run the ${PROJECT_NAME}_package_deb target first"
        exit 1
    fi

    echo "Found $deb_count DEB package(s) in dist/repository"
}

create_tarball_structure() {
    local temp_dir="$DIST_DIR/temp_caos_deb"

    echo "Creating tarball structure in $temp_dir"

    rm -rf "$temp_dir"
    mkdir -p "$temp_dir$CAOS_OPT_DIR"

    # Copy the entire repository structure from dist directory
    if [ -d "$DIST_DIR/repository" ]; then
        echo "Copying repository from $DIST_DIR/repository"
        cp -r "$DIST_DIR/repository" "$temp_dir$CAOS_OPT_DIR/"
    else
        echo "ERROR: $DIST_DIR/repository not found"
        exit 1
    fi

    # Copy install-repository.sh from dist directory
    if [ -f "$DIST_DIR/install-repository.sh" ]; then
        echo "Copying install-repository.sh from $DIST_DIR"
        cp "$DIST_DIR/install-repository.sh" "$temp_dir$CAOS_OPT_DIR/"
    else
        echo "ERROR: $DIST_DIR/install-repository.sh not found"
        exit 1
    fi

    # Copy README.md from dist directory
    if [ -f "$DIST_DIR/README.md" ]; then
        echo "Copying README.md from $DIST_DIR"
        cp "$DIST_DIR/README.md" "$temp_dir$CAOS_OPT_DIR/"
    else
        echo "ERROR: $DIST_DIR/README.md not found"
        exit 1
    fi

    # Copy ENV.md from dist directory
    if [ -f "$DIST_DIR/ENV.md" ]; then
        echo "Copying ENV.md from $DIST_DIR"
        cp "$DIST_DIR/ENV.md" "$temp_dir$CAOS_OPT_DIR/"
    else
        echo "ERROR: $DIST_DIR/ENV.md not found"
        exit 1
    fi

    # Set correct ownership and permissions in temp directory
    echo "Setting secure ownership and permissions in tarball..."

    # Set ownership to root:root for everything
    chown -R root:root "$temp_dir$CAOS_OPT_DIR"

    # Set directory permissions to 755 (readable e traversable da tutti)
    chmod 755 "$temp_dir$CAOS_OPT_DIR"
    find "$temp_dir$CAOS_OPT_DIR" -type d -exec chmod 755 {} \;

    # Set file permissions to 644 (leggibili da tutti)
    find "$temp_dir$CAOS_OPT_DIR" -type f -exec chmod 644 {} \;

    # Set executable permissions to 755 for scripts
    chmod 755 "$temp_dir$CAOS_OPT_DIR/install-repository.sh" 2>/dev/null || true
    chmod 755 "$temp_dir$CAOS_OPT_DIR/repository/update-repo.sh" 2>/dev/null || true
    chmod 755 "$temp_dir$CAOS_OPT_DIR/repository/setup-apt-source.sh" 2>/dev/null || true

    echo "Tarball structure created"
}

create_install_script() {
    local temp_dir="$DIST_DIR/temp_caos_deb"

    # Crea la directory di destinazione
    mkdir -p "$temp_dir$CAOS_OPT_DIR"

    cat > "$temp_dir$CAOS_OPT_DIR/install-repository.sh" << EOF
#!/bin/bash
# ${PROJECT_NAME^^} Repository Installation Script

set -e

CAOS_OPT_DIR="/opt/caosdba"
REPO_DIR="\$CAOS_OPT_DIR/repository"

# Check if running as root
if [ "\$EUID" -ne 0 ]; then
    echo "ERROR: This script must be run as root (use sudo)"
    exit 1
fi

echo "================================================================"
echo "${PROJECT_NAME^^} Repository Installation"
echo "================================================================"

# Verify we're in the correct location
if [ ! -d "\$REPO_DIR" ]; then
    echo "ERROR: Repository directory not found: \$REPO_DIR"
    echo "Make sure the tarball was extracted to /opt/caosdba"
    exit 1
fi

echo "Setting up ${PROJECT_NAME^^} repository from \$REPO_DIR"

# Remove existing APT source files to avoid conflicts
if [ -f "/etc/apt/sources.list.d/caos-local.list" ]; then
    echo "Removing existing caos-local.list to avoid conflicts..."
    rm -f "/etc/apt/sources.list.d/caos-local.list"
fi
if [ -f "/etc/apt/sources.list.d/${PROJECT_NAME}-local.list" ]; then
    echo "Removing existing ${PROJECT_NAME}-local.list to avoid conflicts..."
    rm -f "/etc/apt/sources.list.d/${PROJECT_NAME}-local.list"
fi

# Update repository index
echo "Updating repository index..."
cd "\$REPO_DIR"
if [ -f "./update-repo.sh" ]; then
    ./update-repo.sh
else
    echo "ERROR: update-repo.sh not found in \$REPO_DIR"
    exit 1
fi

# Configure APT
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
echo "  sudo apt install ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}"
echo ""
echo "Or for specific PHP version:"
echo "  sudo apt install ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-8.3"
echo ""
echo "Repository location: \$REPO_DIR"
echo ""
EOF

    chmod 755 "$temp_dir$CAOS_OPT_DIR/install-repository.sh"
    echo "Created install-repository.sh in tarball structure"
}
create_readme() {
    local readme_file="$DIST_DIR/README.md"

    cat > "$readme_file" << EOF
# ${PROJECT_NAME^^} DEB Packages Repository

Version: $VERSION
Database Backend: $DB_BACKEND
Project: $PROJECT_NAME

## Contents

This tarball contains the ${PROJECT_NAME^^} DEB packages and local APT repository.

### Directory Structure

/opt/caosdba/
├── repository/          # Local APT repository
│   ├── *.deb           # DEB packages
│   ├── Packages        # Repository index
│   ├── Packages.gz     # Compressed index
│   ├── Release         # Release info
│   ├── conf/           # Configuration
│   ├── i18n/           # Translations
│   ├── update-repo.sh  # Repository update script
│   └── setup-apt-source.sh # APT configuration script
└── install-repository.sh   # Main installation script

## Installation

### Quick Installation

1. Extract the tarball to /opt:
\`\`\`bash
sudo tar -xzf ${PROJECT_NAME_SANITIZED}-deb-repository-${DB_BACKEND_LOWER}-${VERSION}.tar.gz -C /
\`\`\`

2. Run the installation script:
\`\`\`bash
sudo /opt/caosdba/install-repository.sh
\`\`\`

### Manual Installation

If you prefer manual setup:

1. Extract tarball:
\`\`\`bash
sudo tar -xzf ${PROJECT_NAME_SANITIZED}-deb-repository-${DB_BACKEND_LOWER}-${VERSION}.tar.gz -C /
\`\`\`

2. Update repository index:
\`\`\`bash
cd /opt/caosdba/repository
./update-repo.sh
\`\`\`

3. Configure APT:
\`\`\`bash
./setup-apt-source.sh
\`\`\`

## Usage

After installation, you can install ${PROJECT_NAME^^} packages via APT:

\`\`\`bash
# Install meta-package (recommended)
sudo apt install ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}

# Install specific PHP version
sudo apt install ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-8.3

# See all available packages
apt list ${PROJECT_NAME_SANITIZED}-php*
\`\`\`

## Available Packages

- ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER} - Meta-package
- ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-8.0 - PHP 8.0 extension
- ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-8.1 - PHP 8.1 extension
- ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-8.2 - PHP 8.2 extension
- ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-8.3 - PHP 8.3 extension
- ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}-8.4 - PHP 8.4 extension

## Maintenance

To update the repository after adding new packages:
\`\`\`bash
cd /opt/caosdba/repository
./update-repo.sh
sudo apt update
\`\`\`

## Uninstallation

To remove the repository:

1. Remove APT source:
\`\`\`bash
sudo rm -f /etc/apt/sources.list.d/${PROJECT_NAME}-local.list
\`\`\`

2. Remove repository files:
\`\`\`bash
sudo rm -rf /opt/caosdba
\`\`\`

3. Update APT cache:
\`\`\`bash
sudo apt update
\`\`\`

---
Generated: $(date)
Build: $VERSION
Database: $DB_BACKEND
Project: $PROJECT_NAME
EOF

    echo "Created README.md"
}

create_tarball() {
    local temp_dir="$DIST_DIR/temp_caos_deb"
    local tarball_name="${PROJECT_NAME_SANITIZED}-deb-repository-${DB_BACKEND_LOWER}-${VERSION}.tar.gz"
    local tarball_path="$DIST_DIR/$tarball_name"

    echo "Creating tarball: $tarball_name"

    mkdir -p "$DIST_DIR"
    tar -czf "$tarball_path" -C "$temp_dir" opt

    # Get package info
    local deb_count=$(ls "$temp_dir$CAOS_OPT_DIR/repository"/*.deb 2>/dev/null | wc -l)

    # Cleanup
    rm -rf "$temp_dir"

    echo "Tarball created: $tarball_path"
    echo "Contains $deb_count DEB package(s)"
    echo ""
    echo "To deploy:"
    echo "  sudo tar -xzf $tarball_path -C /"
    echo "  sudo /opt/caosdba/install-repository.sh"
    echo ""
}

create_deployment_readme() {
    local tarball_name="${PROJECT_NAME_SANITIZED}-deb-repository-${DB_BACKEND_LOWER}-${VERSION}.tar.gz"
    local readme_file="$DIST_DIR/${tarball_name}.README.txt"

    cat > "$readme_file" << EOF
To deploy:
1. Extract the tarball:
   sudo tar -xzf $tarball_name -C /

2. Run the installation script:
   sudo /opt/caosdba/install-repository.sh

3. Install ${PROJECT_NAME^^} PHP extension:
   sudo apt install ${PROJECT_NAME_SANITIZED}-php-${DB_BACKEND_LOWER}
EOF

    echo "Created deployment README: ${tarball_name}.README.txt"
}

create_env_documentation() {
    local env_file="$DIST_DIR/ENV.md"

    cat > "$env_file" << EOF
# ${PROJECT_NAME^^} Environment Variables

This document describes all environment variables used by the ${PROJECT_NAME^^} PHP extension.

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

### Basic Redis Configuration

\`\`\`bash
export CAOS_CACHEHOST="redis.example.com"
export CAOS_CACHEPASS="secure_password"
export CAOS_CACHEPOOLSIZEMIN="10"
export CAOS_CACHEPOOLSIZEMAX="50"
export CAOS_CACHEPOOLCONNECTIONLIFETIME="7200"
export CAOS_CACHEPOOLCONNECTIONIDLETIME="600"
\`\`\`

### Database Configuration

\`\`\`bash
export CAOS_DBHOST="mysql.example.com"
export CAOS_DBUSER="app_user"
export CAOS_DBPASS="db_password"
export CAOS_DBNAME="application_db"
export CAOS_DBPOOLSIZEMIN="5"
export CAOS_DBPOOLSIZEMAX="20"
export CAOS_DBCONNECT_TIMEOUT="30"
\`\`\`

### Docker/Container Environment

\`\`\`bash
# Set in your Dockerfile or docker-compose.yml
ENV CAOS_CACHEHOST=redis
ENV CAOS_CACHEPORT=6379
ENV CAOS_DBHOST=mysql
ENV CAOS_DBUSER=app
ENV CAOS_DBPASS=password
ENV CAOS_DBNAME=app_db
\`\`\`

## Notes

Security: Never commit passwords to version control. Use environment-specific configuration files or secret management systems.

Performance: Adjust pool sizes based on your application's concurrency requirements.

Timeouts: Set appropriate timeouts based on your network latency and application requirements.

Validation: Connection validation adds overhead but improves reliability in unstable network environments.

For more information, refer to the ${PROJECT_NAME^^} documentation.
EOF

    echo "Created environment documentation: $env_file"
}


main() {
    echo "Starting DEB repository tarball creation for $PROJECT_NAME..."

    check_prerequisites
    create_install_script
    create_readme
    create_env_documentation
    create_tarball_structure
    create_tarball
    create_deployment_readme

    echo "DEB repository tarball creation completed successfully"
}

main "$@"
