#!/usr/bin/env python3
"""
CAOSDBA Query Validator - Thin wrapper around shared validation module.
Validates YAML query definitions against JSON Schema.
"""

import argparse
import sys
from pathlib import Path
from typing import Optional

from query_tools import QueryValidator, validate_yaml_structure


def main() -> int:
    """Command-line interface for query validator."""
    parser = argparse.ArgumentParser(
        description="Validate CAOSDBA query definitions against JSON Schema"
    )
    
    parser.add_argument(
        "yaml_file",
        nargs="?",
        help="Path to YAML query definitions file (default: queries.yaml in current directory)"
    )
    
    parser.add_argument(
        "--schema",
        "-s",
        help="Path to JSON Schema file (default: schemas/query-schema.json in project root)"
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
    
    args = parser.parse_args()
    
    # Determine YAML file path
    if args.yaml_file:
        yaml_path = Path(args.yaml_file)
    else:
        yaml_path = Path.cwd() / "queries.yaml"
    
    # Determine schema path
    schema_path = Path(args.schema) if args.schema else None
    
    # Diagnose mode
    if args.diagnose:
        validate_yaml_structure(yaml_path)
        return 0
    
    try:
        # Initialize validator using shared module
        validator = QueryValidator(schema_path)
        
        if args.verbose:
            print(f"📋 Schema: {validator.schema_path}")
            print(f"📄 YAML file: {yaml_path}")
            print("─" * 50)
        
        # Validate file using shared module
        if validator.validate_file(yaml_path):
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
