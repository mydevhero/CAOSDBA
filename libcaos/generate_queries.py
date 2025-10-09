#!/usr/bin/env python3
import argparse
import os
from pathlib import Path

def parse_definitions(file_path):
    """Parse the query definitions file"""
    queries = []
    with open(file_path, 'r') as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            # Skip empty lines and comments
            if not line or line.startswith('#'):
                continue

            # Split by pipe
            parts = [part.strip() for part in line.split('|')]
            if len(parts) != 4:
                print(f"Warning: Line {line_num} malformed, expected 4 fields, got {len(parts)}: {line}")
                continue

            return_type, method_name, full_params, call_params = parts
            queries.append({
                'return_type': return_type,
                'method_name': method_name,
                'full_params': full_params,
                'call_params': call_params
            })

    return queries

def generate_query_definition(queries):
    """Generate macro for pure virtual definitions in IQuery"""
    lines = []
    lines.append("// Auto-generated file - DO NOT EDIT MANUALLY")
    lines.append("#ifndef QUERY_DEFINITION_HPP")
    lines.append("#define QUERY_DEFINITION_HPP")
    lines.append("")
    lines.append("#define QUERY_DEFINITION() \\")

    for i, query in enumerate(queries):
        line = f"    virtual {query['return_type']} {query['method_name']}({query['full_params']}) = 0;"
        if i < len(queries) - 1:
            line += " \\"
        lines.append(line)

    lines.append("")
    lines.append("#endif // QUERY_DEFINITION_HPP")
    return '\n'.join(lines)

def generate_query_override(queries):
    """Generate macro for override declarations"""
    lines = []
    lines.append("// Auto-generated file - DO NOT EDIT MANUALLY")
    lines.append("#ifndef QUERY_OVERRIDE_HPP")
    lines.append("#define QUERY_OVERRIDE_HPP")
    lines.append("")
    lines.append("#define QUERY_OVERRIDE() \\")

    for i, query in enumerate(queries):
        line = f"    virtual {query['return_type']} {query['method_name']}({query['full_params']}) override;"
        if i < len(queries) - 1:
            line += " \\"
        lines.append(line)

    lines.append("")
    lines.append("#endif // QUERY_OVERRIDE_HPP")
    return '\n'.join(lines)

def generate_cache_forwarding(queries):
    """Generate forwarding implementations for Cache"""
    lines = []
    lines.append("// Auto-generated file - DO NOT EDIT MANUALLY")
    lines.append("#ifndef CACHE_QUERY_FORWARDING_HPP")
    lines.append("#define CACHE_QUERY_FORWARDING_HPP")
    lines.append("")
    lines.append("#define QUERY_FORWARDING_CACHE() \\")

    for i, query in enumerate(queries):
        # Remove backslashes from the actual C++ code, only keep them for macro continuation
        method_line = f"    {query['return_type']} Cache::{query['method_name']}({query['full_params']}) {{"
        return_line = f"        return this->cache->{query['method_name']}({query['call_params']});"
        end_line = "    }"

        # Combine all parts with backslashes for macro continuation
        full_line = method_line + " \\\n" + return_line + " \\\n" + end_line
        if i < len(queries) - 1:
            full_line += " \\"
        lines.append(full_line)

    lines.append("")
    lines.append("#endif // CACHE_QUERY_FORWARDING_HPP")
    return '\n'.join(lines)

def generate_database_forwarding(queries):
    """Generate forwarding implementations for Database"""
    lines = []
    lines.append("// Auto-generated file - DO NOT EDIT MANUALLY")
    lines.append("#ifndef DATABASE_QUERY_FORWARDING_HPP")
    lines.append("#define DATABASE_QUERY_FORWARDING_HPP")
    lines.append("")
    lines.append("#define QUERY_FORWARDING_DATABASE() \\")

    for i, query in enumerate(queries):
        method_line = f"    {query['return_type']} Database::{query['method_name']}({query['full_params']}) {{"
        return_line = f"        return this->database->{query['method_name']}({query['call_params']});"
        end_line = "    }"

        full_line = method_line + " \\\n" + return_line + " \\\n" + end_line
        if i < len(queries) - 1:
            full_line += " \\"
        lines.append(full_line)

    lines.append("")
    lines.append("#endif // DATABASE_QUERY_FORWARDING_HPP")
    return '\n'.join(lines)

def main():
    parser = argparse.ArgumentParser(description='Generate query interfaces from definitions')
    parser.add_argument('--definitions', required=True, help='Query definitions file')
    parser.add_argument('--output-dir', required=True, help='Output directory for generated files')
    args = parser.parse_args()

    # Check if definitions file exists
    if not os.path.exists(args.definitions):
        print(f"Error: Definitions file not found: {args.definitions}")
        return 1

    # Parse definitions
    queries = parse_definitions(args.definitions)
    if not queries:
        print("No valid queries found in definitions file")
        return 1

    print(f"Found {len(queries)} queries to generate")

    # Create output directory
    output_dir = Path(args.output_dir)
    output_dir.mkdir(exist_ok=True)

    # Generate all files
    (output_dir / "Query_Definition.hpp").write_text(generate_query_definition(queries))
    (output_dir / "Query_Override.hpp").write_text(generate_query_override(queries))
    (output_dir / "Cache_Query_Forwarding.hpp").write_text(generate_cache_forwarding(queries))
    (output_dir / "Database_Query_Forwarding.hpp").write_text(generate_database_forwarding(queries))

    print(f"Files generated in: {output_dir}")
    return 0

if __name__ == '__main__':
    exit(main())
