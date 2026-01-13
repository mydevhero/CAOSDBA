#!/usr/bin/env python3
"""
CAOSDBA Query Validator - Thin wrapper around shared validation module.
Validates YAML query definitions against JSON Schema.
Supports validation of both core and custom query definitions.
"""

import argparse
import sys
from pathlib import Path
from typing import Optional, List, Tuple

from query_tools import QueryValidator, validate_yaml_structure


def validate_directory_structure(base_path: Path) -> List[Tuple[Path, Optional[Path]]]:
    """
    Scan for query definition files in directory structure.

    Returns:
        List of tuples (yaml_path, schema_path) for found query definitions.
        schema_path may be None if schema is missing.
    """
    query_files = []

    if not base_path.exists():
        return query_files

    # Look for queries subdirectories
    queries_dir = base_path / "queries"
    schemas_dir = base_path / "schemas"

    if not queries_dir.exists():
        return query_files

    # Scan all subdirectories in queries/
    for query_subdir in queries_dir.iterdir():
        if query_subdir.is_dir():
            yaml_file = query_subdir / "queries.yaml"

            if yaml_file.exists():
                # Try to find corresponding schema
                schema_file = schemas_dir / query_subdir.name / "queries.json"
                schema_path = schema_file if schema_file.exists() else None

                query_files.append((yaml_file, schema_path))

    return query_files


def main() -> int:
    """Command-line interface for query validator."""
    parser = argparse.ArgumentParser(
        description="Validate CAOSDBA query definitions against JSON Schema. "
                    "Supports both core and custom query definitions."
    )

    parser.add_argument(
        "input_path",
        nargs="?",
        help="Path to query definitions file or directory (default: current directory)"
    )

    parser.add_argument(
        "--schema",
        "-s",
        help="Path to JSON Schema file (optional, uses default if not provided)"
    )

    parser.add_argument(
        "--strict",
        action="store_true",
        help="Exit with error code on validation failure"
    )

    parser.add_argument(
        "--verbose",
        "-v",
        action="store_true",
        help="Show detailed validation information"
    )

    parser.add_argument(
        "--diagnose",
        "-d",
        action="store_true",
        help="Diagnose YAML structure issues"
    )

    parser.add_argument(
        "--scan-directory",
        action="store_true",
        help="Scan directory for all query definition files"
    )

    args = parser.parse_args()

    # Determine input path
    if args.input_path:
        input_path = Path(args.input_path)
    else:
        input_path = Path.cwd()

    # Determine schema path
    schema_path = Path(args.schema) if args.schema else None

    # Diagnose mode
    if args.diagnose:
        if input_path.is_file():
            validate_yaml_structure(input_path)
        else:
            print("❌ Diagnose mode requires a YAML file, not a directory")
            return 1
        return 0

    try:
        # Directory scan mode
        if args.scan_directory or input_path.is_dir():
            print(f"📁 Scanning directory: {input_path}")

            query_files = validate_directory_structure(input_path)

            if not query_files:
                print("❌ No query definition files found")
                return 1

            all_valid = True

            for yaml_path, schema_path in query_files:
                print(f"\n{'─' * 50}")
                print(f"📋 Validating: {yaml_path}")

                if schema_path:
                    print(f"📄 Using schema: {schema_path}")
                    validator = QueryValidator(schema_path)
                else:
                    print(f"⚠️  No schema found, using default")
                    validator = QueryValidator()

                if validator.validate_file(yaml_path):
                    print("✅ Validation passed")
                else:
                    print("❌ Validation failed")
                    all_valid = False

            if args.strict and not all_valid:
                return 1
            return 0 if all_valid else 0  # Non-strict mode returns 0 even on failure

        # Single file mode
        else:
            # Initialize validator using shared module
            validator = QueryValidator(schema_path)

            if args.verbose:
                print(f"📋 Schema: {validator.schema_path}")
                print(f"📄 YAML file: {input_path}")
                print("─" * 50)

            # Validate file using shared module
            if validator.validate_file(input_path):
                print("✅ Validation passed")
                return 0
            else:
                if args.strict:
                    return 1
                return 0  # Non-strict mode returns 0 even on failure

    except FileNotFoundError as e:
        print(f"❌ File error: {e}", file=sys.stderr)
        return 1
    except Exception as e:
        print(f"❌ Unexpected error: {e}", file=sys.stderr)
        if args.verbose:
            import traceback
            traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())
