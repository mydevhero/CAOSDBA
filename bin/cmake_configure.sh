#!/bin/bash

if [ "$#" -lt 2 ]; then
    echo "Error: at least two arguments expected"
    echo "Use: $0 <POSTGRESQL|MYSQL|MARIADB> <BINDING|CROWCPP|STANDALONE> [NODEJS|PHP|PYTHON]"
    exit 1
fi

DB_BACKEND="$1"
PROJECT_TYPE="$2"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$SCRIPT_DIR/.."
RELEASE_DIR="$SOURCE_DIR/build/release"

mkdir -p "${RELEASE_DIR}"

if [ -d "${RELEASE_DIR}" ]; then
    echo "Created directory ${RELEASE_DIR}"
    cd "${RELEASE_DIR}"
else
    echo "Can't change directory to ${RELEASE_DIR}"
    exit 1;
fi

if [ "$PROJECT_TYPE" == "BINDING" ]; then
    if [ "$#" -ne 3 ]; then
        echo "Error: BINDING requires a third argument"
        echo "Use: $0 <POSTGRESQL|MYSQL|MARIADB> <BINDING|CROWCPP|STANDALONE> <NODEJS|PHP|PYTHON>"
        exit 1
    fi

    BINDING_LANGUAGE="$3"

    cmake -G Ninja -DCAOS_DB_BACKEND=$DB_BACKEND -DCAOS_PROJECT_TYPE=$PROJECT_TYPE -DCAOS_BINDING_LANGUAGE=$BINDING_LANGUAGE ../../
else
    cmake -G Ninja -DCAOS_DB_BACKEND=$DB_BACKEND -DCAOS_PROJECT_TYPE=$PROJECT_TYPE ../../
fi
