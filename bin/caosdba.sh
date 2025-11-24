#!/bin/bash

# Function to display script usage
usage() {
    echo "Usage: $0 --db=<database_type> --type=<project_type>"
    echo "       $0 --purge"
    echo "Database types: postgresql, mysql, mariadb"
    echo "Project types: standalone, binding, crowcpp"
    exit 1
}

# Function to purge project files
purge_project() {
    echo "WARNING: This will delete all project files created by this script."
    echo "This includes:"
    echo "  - $CMAKE_SOURCE_DIR/src/ (entire directory)"
    echo "  - Files in $CMAKE_SOURCE_DIR/cmake/"
    echo ""
    read -p "Are you sure you want to continue? (y/N): " confirm

    case $confirm in
        [yY]|[yY][eE][sS])
            echo "Purging project files..."

            # Remove src directory
            if [ -d "$CMAKE_SOURCE_DIR/src" ]; then
                rm -rf "$CMAKE_SOURCE_DIR/src"
                echo "Removed $CMAKE_SOURCE_DIR/src"
            else
                echo "src directory not found, skipping"
            fi

            # Remove cmake files created by this script
            cmake_files=("db_backend.cmake" "custom_code.cmake" "app.cmake")
            for file in "${cmake_files[@]}"; do
                if [ -f "$CMAKE_SOURCE_DIR/cmake/$file" ]; then
                    rm "$CMAKE_SOURCE_DIR/cmake/$file"
                    echo "Removed $CMAKE_SOURCE_DIR/cmake/$file"
                fi
            done

            echo "Purge completed successfully"
            ;;
        *)
            echo "Purge cancelled"
            exit 0
            ;;
    esac
}

# Variables
DB_TYPE=""
PROJECT_TYPE=""
PURGE_FLAG=false
CMAKE_SOURCE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
TEMPLATE_DIR="$CMAKE_SOURCE_DIR/template"

# Parse arguments
for arg in "$@"; do
    case $arg in
        --db=*)
        DB_TYPE="${arg#*=}"
        shift
        ;;
        --type=*)
        PROJECT_TYPE="${arg#*=}"
        shift
        ;;
        --purge)
        PURGE_FLAG=true
        shift
        ;;
        *)
        usage
        ;;
    esac
done

# Handle purge mode
if [ "$PURGE_FLAG" = true ]; then
    if [ -n "$DB_TYPE" ] || [ -n "$PROJECT_TYPE" ]; then
        echo "Error: --purge cannot be combined with other options"
        usage
    fi
    purge_project
    exit 0
fi

# Validate arguments for normal mode
if [ -z "$DB_TYPE" ] || [ -z "$PROJECT_TYPE" ]; then
    echo "Error: Both --db and --type parameters are required"
    usage
fi

# Validate database type and set proper directory name
case $DB_TYPE in
    postgresql)
        DB_DIR="PostgreSQL"
        ;;
    mysql)
        DB_DIR="MySQL"
        ;;
    mariadb)
        DB_DIR="MariaDB"
        ;;
    *)
        echo "Error: Invalid database type '$DB_TYPE'"
        usage
        ;;
esac

# Validate project type
case $PROJECT_TYPE in
    standalone|binding|crowcpp)
        ;;
    *)
        echo "Error: Invalid project type '$PROJECT_TYPE'"
        usage
        ;;
esac

# Check if src directory already exists
if [ -d "$CMAKE_SOURCE_DIR/src" ]; then
    echo "Error: Directory $CMAKE_SOURCE_DIR/src already exists. Cannot proceed."
    echo "Use --purge to remove existing project files first."
    exit 1
fi

# Set template subdirectory based on project type
TEMPLATE_SUBDIR="$TEMPLATE_DIR/$PROJECT_TYPE"

# Check if template subdirectory exists
if [ ! -d "$TEMPLATE_SUBDIR" ]; then
    echo "Error: Template directory $TEMPLATE_SUBDIR does not exist"
    exit 1
fi

# Create cmake directory
CMAKE_DIR="$CMAKE_SOURCE_DIR/cmake"
mkdir -p "$CMAKE_DIR"

# Create db_backend.cmake with DB_BACKEND in uppercase
DB_BACKEND_UPPER=$(echo "$DB_TYPE" | tr '[:lower:]' '[:upper:]')
cat > "$CMAKE_DIR/db_backend.cmake" << EOF
set(DB_BACKEND $DB_BACKEND_UPPER)
EOF
echo "Created $CMAKE_DIR/db_backend.cmake with DB_BACKEND=$DB_BACKEND_UPPER"

# Copy template files
echo "Copying template files from $TEMPLATE_SUBDIR..."

# Copy custom_code.cmake
cp "$TEMPLATE_SUBDIR/cmake/custom_code.cmake" "$CMAKE_DIR/"
echo "Copied custom_code.cmake"

# Copy Redis Cache Query.hpp
REDIS_SRC="$TEMPLATE_SUBDIR/src/include/Cache/Redis/Query.hpp"
REDIS_DEST="$CMAKE_SOURCE_DIR/src/include/Cache/Redis/Query.hpp"
mkdir -p "$(dirname "$REDIS_DEST")"
cp "$REDIS_SRC" "$REDIS_DEST"
echo "Copied Redis Query.hpp"

# Copy Database Query.hpp based on selected database
DB_SRC="$TEMPLATE_SUBDIR/src/include/Database/$DB_DIR/Query.hpp"
DB_DEST="$CMAKE_SOURCE_DIR/src/include/Database/$DB_DIR/Query.hpp"
mkdir -p "$(dirname "$DB_DEST")"
cp "$DB_SRC" "$DB_DEST"
echo "Copied $DB_DIR Database Query.hpp"

# Handle project type specific files
case $PROJECT_TYPE in
    standalone)
        # Copy app.cmake
        cp "$TEMPLATE_SUBDIR/cmake/app.cmake" "$CMAKE_DIR/"
        echo "Copied app.cmake"

        # Copy main.cpp
        mkdir -p "$CMAKE_SOURCE_DIR/src"
        cp "$TEMPLATE_SUBDIR/src/main.cpp" "$CMAKE_SOURCE_DIR/src/"
        echo "Copied main.cpp"
        ;;
    binding|crowcpp)
        # Copy all files from the project type template
        echo "Copying all files from $PROJECT_TYPE template..."
        cp -r "$TEMPLATE_SUBDIR"/* "$CMAKE_SOURCE_DIR/"
        echo "Copied $PROJECT_TYPE project structure"
        ;;
esac

echo ""
echo "Setup completed successfully for:"
echo "  Database: $DB_TYPE ($DB_DIR)"
echo "  Project type: $PROJECT_TYPE"
echo "  Source directory: $CMAKE_SOURCE_DIR"
