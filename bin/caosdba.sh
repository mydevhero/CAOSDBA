#!/bin/bash

# Function to display script usage
usage() {
    echo "Usage: $0 --init"
    echo "       $0 --purge"
    echo ""
    echo "Options:"
    echo "  --init     Initialize project from CMake configuration files"
    echo "  --purge    Remove all generated project files"
    echo ""
    echo "Configuration is read from:"
    echo "  - cmake/db_backend.cmake (DB_BACKEND)"
    echo "  - cmake/project_type.cmake (PROJECT_TYPE)"
    echo "  - cmake/binding_language.cmake (BINDING_LANGUAGE, if PROJECT_TYPE is BINDING)"
    exit 1
}

# Function to purge project files
purge_project() {
    echo "WARNING: This will delete all project files created by this script."
    echo "This includes:"
    echo "  - $CMAKE_SOURCE_DIR/src/ (entire directory)"
    echo "  - $CMAKE_SOURCE_DIR/scripts/ (entire directory)"
    echo "  - Files in $CMAKE_SOURCE_DIR/cmake/"
    echo ""
    read -p "Are you sure you want to continue? (y/N): " confirm

    case $confirm in
        [yY])
            echo "Purging project files..."

            # Remove src directory
            if [ -d "$CMAKE_SOURCE_DIR/src" ]; then
                rm -rf "$CMAKE_SOURCE_DIR/src"
                echo "Removed $CMAKE_SOURCE_DIR/src"
            else
                echo "src directory not found, skipping"
            fi

            # Remove scripts directory
            if [ -d "$CMAKE_SOURCE_DIR/scripts" ]; then
                rm -rf "$CMAKE_SOURCE_DIR/scripts"
                echo "Removed $CMAKE_SOURCE_DIR/scripts"
            else
                echo "scripts directory not found, skipping"
            fi

            # Remove cmake files created by this script
            cmake_files=("type_code.cmake" "app.cmake" "app_debug.cmake" "project_type.cmake" "binding_language.cmake" "db_backend.cmake")
            for file in "${cmake_files[@]}"; do
                if [ -f "$CMAKE_SOURCE_DIR/cmake/$file" ]; then
                    rm "$CMAKE_SOURCE_DIR/cmake/$file"
                    echo "Removed $CMAKE_SOURCE_DIR/cmake/$file"
                fi
            done

            echo "Purge completed successfully"
            return 0
            ;;
        *)
            echo "Purge cancelled"
            exit 0
            ;;
    esac
}

# Function to read CMake variable from file
read_cmake_var() {
    local file=$1
    local var_name=$2

    if [ ! -f "$file" ]; then
        return 1
    fi

    # Extract value from set(VAR_NAME VALUE)
    local value=$(grep "^set($var_name" "$file" | sed -n "s/^set($var_name \(.*\)).*$/\1/p" | tr -d ' ')

    if [ -z "$value" ]; then
        return 1
    fi

    echo "$value"
    return 0
}

# Function to initialize project
init_project() {
    # Check if project is already initialized
    if [ -d "$CMAKE_SOURCE_DIR/src" ] || [ -f "$CMAKE_SOURCE_DIR/cmake/app.cmake" ]; then
        echo "Project already initialized, skipping setup"
        exit 0
    fi

    # Read database backend configuration
    DB_BACKEND=$(read_cmake_var "$CMAKE_SOURCE_DIR/cmake/db_backend.cmake" "DB_BACKEND")
    if [ $? -ne 0 ] || [ -z "$DB_BACKEND" ]; then
        echo "Error: Could not read DB_BACKEND from cmake/db_backend.cmake"
        echo "Make sure the file exists and contains: set(DB_BACKEND <value>)"
        exit 1
    fi

    # Read project type configuration
    PROJECT_TYPE=$(read_cmake_var "$CMAKE_SOURCE_DIR/cmake/project_type.cmake" "PROJECT_TYPE")
    if [ $? -ne 0 ] || [ -z "$PROJECT_TYPE" ]; then
        echo "Error: Could not read PROJECT_TYPE from cmake/project_type.cmake"
        echo "Make sure the file exists and contains: set(PROJECT_TYPE <value>)"
        exit 1
    fi

    # Validate database type and set proper directory name
    case $DB_BACKEND in
        POSTGRESQL)
            DB_DIR="PostgreSQL"
            ;;
        MYSQL)
            DB_DIR="MySQL"
            ;;
        MARIADB)
            DB_DIR="MariaDB"
            ;;
        *)
            echo "Error: Invalid database type '$DB_BACKEND' in db_backend.cmake"
            echo "Valid options: POSTGRESQL, MYSQL, MARIADB"
            exit 1
            ;;
    esac

    # Validate project type
    case $PROJECT_TYPE in
        STANDALONE)
            TEMPLATE_SUBDIR="$TEMPLATE_DIR/standalone"
            ;;
        BINDING)
            # Read binding language for BINDING type
            BINDING_LANGUAGE=$(read_cmake_var "$CMAKE_SOURCE_DIR/cmake/binding_language.cmake" "BINDING_LANGUAGE")
            if [ $? -ne 0 ] || [ -z "$BINDING_LANGUAGE" ]; then
                echo "Error: Could not read BINDING_LANGUAGE from cmake/binding_language.cmake"
                echo "BINDING_LANGUAGE is required when PROJECT_TYPE is BINDING"
                exit 1
            fi

            # Validate and convert binding language to lowercase for directory
            case $BINDING_LANGUAGE in
                PHP)
                    LANG_DIR="php"
                    ;;
                NODE)
                    LANG_DIR="node"
                    ;;
                PYTHON)
                    LANG_DIR="python"
                    ;;
                *)
                    echo "Error: Invalid binding language '$BINDING_LANGUAGE' in binding_language.cmake"
                    echo "Valid options: PHP, NODE, PYTHON"
                    exit 1
                    ;;
            esac

            TEMPLATE_SUBDIR="$TEMPLATE_DIR/bindings/$LANG_DIR"
            ;;
        *)
            echo "Error: Invalid project type '$PROJECT_TYPE' in project_type.cmake"
            echo "Valid options: STANDALONE, BINDING"
            exit 1
            ;;
    esac

    # Check if template subdirectory exists
    if [ ! -d "$TEMPLATE_SUBDIR" ]; then
        echo "Error: Template directory $TEMPLATE_SUBDIR does not exist"
        exit 1
    fi

    echo "Initializing project with configuration:"
    echo "  Database: $DB_BACKEND ($DB_DIR)"
    echo "  Project type: $PROJECT_TYPE"
    if [ "$PROJECT_TYPE" = "BINDING" ]; then
        echo "  Language: $BINDING_LANGUAGE"
    fi
    echo ""

    # Create cmake directory if not exists
    CMAKE_DIR="$CMAKE_SOURCE_DIR/cmake"
    mkdir -p "$CMAKE_DIR"

    # Copy template files based on project type
    case $PROJECT_TYPE in
        STANDALONE)
            # Copy type_code.cmake
            cp "$TEMPLATE_SUBDIR/cmake/type_code.cmake" "$CMAKE_DIR/"
            echo "Copied type_code.cmake"

            # Copy app.cmake
            cp "$TEMPLATE_SUBDIR/cmake/app.cmake" "$CMAKE_DIR/"
            echo "Copied app.cmake"

            # Copy app_debug.cmake
            cp "$TEMPLATE_SUBDIR/cmake/app_debug.cmake" "$CMAKE_DIR/"
            echo "Copied app_debug.cmake"

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

            # Copy main.cpp
            mkdir -p "$CMAKE_SOURCE_DIR/src"
            cp "$TEMPLATE_SUBDIR/src/main.cpp" "$CMAKE_SOURCE_DIR/src/"
            echo "Copied main.cpp"
            ;;

        BINDING)
            echo "Copying binding template structure..."

            # Copy everything except src/include directory
            for item in "$TEMPLATE_SUBDIR"/*; do
                item_name=$(basename "$item")
                if [ "$item_name" != "src" ]; then
                    cp -r "$item" "$CMAKE_SOURCE_DIR/"
                    echo "Copied $item_name"
                fi
            done

            # Handle src directory separately
            if [ -d "$TEMPLATE_SUBDIR/src" ]; then
                # Copy everything in src except include
                for item in "$TEMPLATE_SUBDIR/src"/*; do
                    item_name=$(basename "$item")
                    if [ "$item_name" != "include" ]; then
                        mkdir -p "$CMAKE_SOURCE_DIR/src"
                        cp -r "$item" "$CMAKE_SOURCE_DIR/src/"
                        echo "Copied src/$item_name"
                    fi
                done

                # Handle src/include directory
                if [ -d "$TEMPLATE_SUBDIR/src/include" ]; then
                    INCLUDE_SRC="$TEMPLATE_SUBDIR/src/include"
                    INCLUDE_DEST="$CMAKE_SOURCE_DIR/src/include"
                    mkdir -p "$INCLUDE_DEST"

                    # Copy files directly in include/ (like extension_config.h.in)
                    find "$INCLUDE_SRC" -maxdepth 1 -type f -exec cp {} "$INCLUDE_DEST/" \;

                    # Copy Cache directory
                    if [ -d "$INCLUDE_SRC/Cache" ]; then
                        cp -r "$INCLUDE_SRC/Cache" "$INCLUDE_DEST/"
                        echo "Copied src/include/Cache"
                    fi

                    # Copy only the selected database directory
                    if [ -d "$INCLUDE_SRC/Database/$DB_DIR" ]; then
                        mkdir -p "$INCLUDE_DEST/Database"
                        cp -r "$INCLUDE_SRC/Database/$DB_DIR" "$INCLUDE_DEST/Database/"
                        echo "Copied src/include/Database/$DB_DIR"
                    else
                        echo "Warning: Database template for $DB_DIR not found"
                    fi
                fi
            fi
            ;;
    esac

    echo ""
    echo "Project initialization completed successfully!"
    echo "  Source directory: $CMAKE_SOURCE_DIR"
}

# Variables
INIT_FLAG=false
PURGE_FLAG=false
CMAKE_SOURCE_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
TEMPLATE_DIR="$CMAKE_SOURCE_DIR/template"

# Parse arguments
if [ $# -eq 0 ]; then
    usage
fi

for arg in "$@"; do
    case $arg in
        --init)
            INIT_FLAG=true
            shift
            ;;
        --purge)
            PURGE_FLAG=true
            shift
            ;;
        *)
            echo "Error: Unknown option '$arg'"
            usage
            ;;
    esac
done

# Check for mutually exclusive flags
if [ "$INIT_FLAG" = true ] && [ "$PURGE_FLAG" = true ]; then
    echo "Error: Options --init and --purge are mutually exclusive"
    usage
fi

# Execute based on flag
if [ "$PURGE_FLAG" = true ]; then
    purge_project
    exit $?
elif [ "$INIT_FLAG" = true ]; then
    init_project
    exit 0
else
    echo "Error: No option specified"
    usage
fi
