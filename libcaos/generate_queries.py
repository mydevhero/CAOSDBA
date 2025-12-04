#!/usr/bin/env python3
import argparse
import logging
from pathlib import Path

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)
logger = logging.getLogger(__name__)

def parse_definitions(file_path):
    """Parse the query definitions file"""
    queries = []
    logger.info(f"Parsing definitions from: {file_path}")

    with open(file_path, 'r') as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            # Skip empty lines and comments
            if not line or line.startswith('#'):
                continue

            # Split by pipe
            parts = [part.strip() for part in line.split('|')]
            if len(parts) != 7:
                logger.warning(f"Line {line_num} malformed, expected 7 fields, got {len(parts)}: {line}")
                continue

            return_type, method_name, full_params, call_params, auth_type, env_var_name, key_behavior = parts
            queries.append({
                'return_type': return_type,
                'method_name': method_name,
                'full_params': full_params,
                'call_params': call_params,
                'auth_type': auth_type,
                'env_var_name': env_var_name,
                'key_behavior': key_behavior
            })
            logger.debug(f"Parsed query: {method_name}")

    logger.info(f"Successfully parsed {len(queries)} queries")
    return queries

def generate_query_definition(queries):
    """Generate macro for pure virtual definitions in IQuery"""
    logger.debug("Generating Query_Definition.hpp")
    lines = []
    lines.append("// Auto-generated file - DO NOT EDIT MANUALLY")
    lines.append("#ifndef QUERY_DEFINITION_HPP")
    lines.append("#define QUERY_DEFINITION_HPP")
    lines.append("")

    if queries:
        lines.append("#define QUERY_DEFINITION() \\")
        for i, query in enumerate(queries):
            line = f"    virtual {query['return_type']} {query['method_name']}({query['full_params']}) = 0;"
            if i < len(queries) - 1:
                line += " \\"
            lines.append(line)
    else:
        lines.append("// No queries defined")
        lines.append("#define QUERY_DEFINITION()")

    lines.append("")
    lines.append("#endif // QUERY_DEFINITION_HPP")
    return '\n'.join(lines)

def generate_query_override(queries):
    """Generate macro for override declarations"""
    logger.debug("Generating Query_Override.hpp")
    lines = []
    lines.append("// Auto-generated file - DO NOT EDIT MANUALLY")
    lines.append("#ifndef QUERY_OVERRIDE_HPP")
    lines.append("#define QUERY_OVERRIDE_HPP")
    lines.append("")

    if queries:
        lines.append("#define QUERY_OVERRIDE() \\")
        for i, query in enumerate(queries):
            line = f"    virtual {query['return_type']} {query['method_name']}({query['full_params']}) override;"
            if i < len(queries) - 1:
                line += " \\"
            lines.append(line)
    else:
        lines.append("// No queries defined")
        lines.append("#define QUERY_OVERRIDE()")

    lines.append("")
    lines.append("#endif // QUERY_OVERRIDE_HPP")
    return '\n'.join(lines)

def generate_cache_forwarding(queries):
    """Generate forwarding implementations for Cache"""
    logger.debug("Generating Cache_Query_Forwarding.hpp")
    lines = []
    lines.append("// Auto-generated file - DO NOT EDIT MANUALLY")
    lines.append("#ifndef CACHE_QUERY_FORWARDING_HPP")
    lines.append("#define CACHE_QUERY_FORWARDING_HPP")
    lines.append("")

    if queries:
        lines.append("#define QUERY_FORWARDING_CACHE() \\")
        for i, query in enumerate(queries):
            method_line = f"    {query['return_type']} Cache::{query['method_name']}({query['full_params']}) {{"
            return_line = f"        return this->cache->{query['method_name']}({query['call_params']});"
            end_line = "    }"

            full_line = method_line + " \\\n" + return_line + " \\\n" + end_line
            if i < len(queries) - 1:
                full_line += " \\"
            lines.append(full_line)
    else:
        lines.append("// No queries defined")
        lines.append("#define QUERY_FORWARDING_CACHE()")

    lines.append("")
    lines.append("#endif // CACHE_QUERY_FORWARDING_HPP")
    return '\n'.join(lines)

def generate_database_forwarding(queries):
    """Generate forwarding implementations for Database"""
    logger.debug("Generating Database_Query_Forwarding.hpp")
    lines = []
    lines.append("// Auto-generated file - DO NOT EDIT MANUALLY")
    lines.append("#ifndef DATABASE_QUERY_FORWARDING_HPP")
    lines.append("#define DATABASE_QUERY_FORWARDING_HPP")
    lines.append("")

    if queries:
        lines.append("#define QUERY_FORWARDING_DATABASE() \\")
        for i, query in enumerate(queries):
            method_line = f"    {query['return_type']} Database::{query['method_name']}({query['full_params']}) {{"
            return_line = f"        return this->database->{query['method_name']}({query['call_params']});"
            end_line = "    }"

            full_line = method_line + " \\\n" + return_line + " \\\n" + end_line
            if i < len(queries) - 1:
                full_line += " \\"
            lines.append(full_line)
    else:
        lines.append("// No queries defined")
        lines.append("#define QUERY_FORWARDING_DATABASE()")

    lines.append("")
    lines.append("#endif // DATABASE_QUERY_FORWARDING_HPP")
    return '\n'.join(lines)

def generate_auth_config(queries):
    """Generate authentication configuration"""
    logger.debug("Generating AuthConfig.hpp")
    lines = []
    lines.append("// Auto-generated file - DO NOT EDIT MANUALLY")
    lines.append("#ifndef AUTH_CONFIG_HPP")
    lines.append("#define AUTH_CONFIG_HPP")
    lines.append("")
    lines.append("#include <string>")
    lines.append("#include <unordered_map>")
    lines.append("#include <array>")
    lines.append("")
    lines.append("enum class AuthType { TOKEN };")
    lines.append("")
    lines.append("struct AuthConfig {")
    lines.append("    AuthType type;")
    lines.append("    std::string envVar;")
    lines.append("};")
    lines.append("")

    if queries:
        # Generate AutoToken array
        auto_token_vars = []
        for query in queries:
            if query['auth_type'] == 'TOKEN' and query['key_behavior'] == 'AUTO':
                auto_token_vars.append(query['env_var_name'])

        if auto_token_vars:
            lines.append(f"static constexpr std::array<const char*, {len(auto_token_vars)}> AutoToken = {{")
            for i, env_var in enumerate(auto_token_vars):
                line = f'    "{env_var}"'
                if i < len(auto_token_vars) - 1:
                    line += ","
                lines.append(line)
            lines.append("};")
            lines.append("")

        # Generate QUERY_AUTH_MAP
        lines.append("static std::unordered_map<std::string, AuthConfig> QUERY_AUTH_MAP = {")
        for i, query in enumerate(queries):
            if query['auth_type']:  # Only add if auth_type is specified
                auth_type_enum = f"AuthType::{query['auth_type']}"
                env_var = query['env_var_name']
                line = f'    {{"{query["method_name"]}", {{{auth_type_enum}, "{env_var}"}}}}'
                if i < len(queries) - 1:
                    line += ","
                lines.append(line)
        lines.append("};")
    else:
        lines.append("// No queries defined")
        lines.append("static constexpr std::array<const char*, 0> AutoToken = {};")
        lines.append("static std::unordered_map<std::string, AuthConfig> QUERY_AUTH_MAP = {};")

    lines.append("")
    lines.append("#endif // AUTH_CONFIG_HPP")
    return '\n'.join(lines)

def generate_cmake_config(queries):
    """Generate CMake configuration with compile definitions"""
    logger.debug("Generating Query_Config.cmake")
    lines = []
    lines.append("# Auto-generated CMake configuration - DO NOT EDIT MANUALLY")
    lines.append("")

    if queries:
        lines.append("# Query definitions found")
        lines.append("add_compile_definitions(")
        for query in queries:
            lines.append(f"    QUERY_EXISTS_{query['method_name']}")
        lines.append(")")
        lines.append("")
        lines.append("# Set individual query flags")
        for query in queries:
            lines.append(f"set(QUERY_EXISTS_{query['method_name']} TRUE)")
    else:
        lines.append("# No query definitions found")
        lines.append("# target_compile_definitions will be empty")
        lines.append("# No individual query flags to set")

    lines.append("")
    return '\n'.join(lines)

def main():
    parser = argparse.ArgumentParser(description='Generate query interfaces from definitions')
    parser.add_argument('--definitions', required=True, help='Query definitions file')
    parser.add_argument('--output-dir', required=True, help='Output directory for generated files')

    # Flag OPTIONAL - se non passate, default False
    parser.add_argument('--caos-example-query', action='store_true', default=False,
                       help='Enable IQuery_Example_echoString')
    parser.add_argument('--caos-template-query', action='store_true', default=False,
                       help='Enable IQuery_Template_echoString')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Enable verbose debug logging')

    args = parser.parse_args()

    # Set logging level
    if args.verbose:
        logger.setLevel(logging.DEBUG)
        logger.debug("Verbose logging enabled")

    logger.info("=== Query Code Generator ===")

    # Check if definitions file exists
    definitions_path = Path(args.definitions)
    if not definitions_path.exists():
        logger.error(f"Definitions file not found: {definitions_path}")
        return 1

    # Parse ALL definitions
    all_queries = parse_definitions(args.definitions)

    # Controlla quali flag sono stati passati
    example_passed = args.caos_example_query
    template_passed = args.caos_template_query

    logger.info(f"Flag --caos-example-query passed: {example_passed}")
    logger.info(f"Flag --caos-template-query passed: {template_passed}")

    # Filter queries based on rules
    enabled_queries = []

    for query in all_queries:
        method_name = query['method_name']

        # Regola 1: Se è IQuery_Example_echoString, abilita SOLO se --caos-example-query è passato
        if method_name == 'IQuery_Example_echoString':
            if example_passed:
                enabled_queries.append(query)
                logger.info("✅ Enabling IQuery_Example_echoString (flag --caos-example-query passed)")
            else:
                logger.info("❌ Skipping IQuery_Example_echoString (flag --caos-example-query not passed)")

        # Regola 2: Se è IQuery_Template_echoString, abilita SOLO se --caos-template-query è passato
        elif method_name == 'IQuery_Template_echoString':
            if template_passed:
                enabled_queries.append(query)
                logger.info("✅ Enabling IQuery_Template_echoString (flag --caos-template-query passed)")
            else:
                logger.info("❌ Skipping IQuery_Template_echoString (flag --caos-template-query not passed)")

        # Regola 3: Per TUTTE le altre query, abilita SEMPRE
        else:
            enabled_queries.append(query)
            logger.debug(f"✅ Enabling other query: {method_name}")

    logger.info(f"Total queries found: {len(all_queries)}")
    logger.info(f"Queries enabled: {len(enabled_queries)}")

    # Verifica se abbiamo le due query speciali
    special_methods = ['IQuery_Example_echoString', 'IQuery_Template_echoString']
    for method in special_methods:
        enabled = any(q['method_name'] == method for q in enabled_queries)
        logger.debug(f"{method} enabled: {enabled}")

    # Count auto tokens only from enabled queries
    auto_tokens = [q for q in enabled_queries if q['auth_type'] == 'TOKEN' and q['key_behavior'] == 'AUTO']
    logger.info(f"Found {len(auto_tokens)} auto-generated tokens")

    # Create output directory
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    logger.info(f"Output directory: {output_dir.absolute()}")

    # Generate all files only with enabled queries
    logger.info("Generating files...")

    (output_dir / "Query_Definition.hpp").write_text(generate_query_definition(enabled_queries))
    logger.info("✓ Generated: Query_Definition.hpp")

    (output_dir / "Query_Override.hpp").write_text(generate_query_override(enabled_queries))
    logger.info("✓ Generated: Query_Override.hpp")

    (output_dir / "Cache_Query_Forwarding.hpp").write_text(generate_cache_forwarding(enabled_queries))
    logger.info("✓ Generated: Cache_Query_Forwarding.hpp")

    (output_dir / "Database_Query_Forwarding.hpp").write_text(generate_database_forwarding(enabled_queries))
    logger.info("✓ Generated: Database_Query_Forwarding.hpp")

    (output_dir / "AuthConfig.hpp").write_text(generate_auth_config(enabled_queries))
    logger.info("✓ Generated: AuthConfig.hpp")

    (output_dir / "Query_Config.cmake").write_text(generate_cmake_config(enabled_queries))
    logger.info("✓ Generated: Query_Config.cmake")

    logger.info("✅ Code generation completed successfully")
    return 0

if __name__ == '__main__':
    exit(main())



# #!/usr/bin/env python3
# import argparse
# import os
# from pathlib import Path

# def parse_definitions(file_path):
#     """Parse the query definitions file"""
#     queries = []
#     with open(file_path, 'r') as f:
#         for line_num, line in enumerate(f, 1):
#             line = line.strip()
#             # Skip empty lines and comments
#             if not line or line.startswith('#'):
#                 continue

#             # Split by pipe
#             parts = [part.strip() for part in line.split('|')]
#             if len(parts) != 7:
#                 print("Warning: Line {line_num} malformed, expected 7 fields, got {len(parts)}: {line}")
#                 continue

#             return_type, method_name, full_params, call_params, auth_type, env_var_name, key_behavior = parts
#             queries.append({
#                 'return_type': return_type,
#                 'method_name': method_name,
#                 'full_params': full_params,
#                 'call_params': call_params,
#                 'auth_type': auth_type,
#                 'env_var_name': env_var_name,
#                 'key_behavior': key_behavior
#             })

#     return queries

# def generate_query_definition(queries):
#     """Generate macro for pure virtual definitions in IQuery"""
#     lines = []
#     lines.append("// Auto-generated file - DO NOT EDIT MANUALLY")
#     lines.append("#ifndef QUERY_DEFINITION_HPP")
#     lines.append("#define QUERY_DEFINITION_HPP")
#     lines.append("")

#     if queries:
#         lines.append("#define QUERY_DEFINITION() \\")
#         for i, query in enumerate(queries):
#             line = f"    virtual {query['return_type']} {query['method_name']}({query['full_params']}) = 0;"
#             if i < len(queries) - 1:
#                 line += " \\"
#             lines.append(line)
#     else:
#         lines.append("// No queries defined")
#         lines.append("#define QUERY_DEFINITION()")

#     lines.append("")
#     lines.append("#endif // QUERY_DEFINITION_HPP")
#     return '\n'.join(lines)

# def generate_query_override(queries):
#     """Generate macro for override declarations"""
#     lines = []
#     lines.append("// Auto-generated file - DO NOT EDIT MANUALLY")
#     lines.append("#ifndef QUERY_OVERRIDE_HPP")
#     lines.append("#define QUERY_OVERRIDE_HPP")
#     lines.append("")

#     if queries:
#         lines.append("#define QUERY_OVERRIDE() \\")
#         for i, query in enumerate(queries):
#             line = f"    virtual {query['return_type']} {query['method_name']}({query['full_params']}) override;"
#             if i < len(queries) - 1:
#                 line += " \\"
#             lines.append(line)
#     else:
#         lines.append("// No queries defined")
#         lines.append("#define QUERY_OVERRIDE()")

#     lines.append("")
#     lines.append("#endif // QUERY_OVERRIDE_HPP")
#     return '\n'.join(lines)

# def generate_cache_forwarding(queries):
#     """Generate forwarding implementations for Cache"""
#     lines = []
#     lines.append("// Auto-generated file - DO NOT EDIT MANUALLY")
#     lines.append("#ifndef CACHE_QUERY_FORWARDING_HPP")
#     lines.append("#define CACHE_QUERY_FORWARDING_HPP")
#     lines.append("")

#     if queries:
#         lines.append("#define QUERY_FORWARDING_CACHE() \\")
#         for i, query in enumerate(queries):
#             method_line = f"    {query['return_type']} Cache::{query['method_name']}({query['full_params']}) {{"
#             return_line = f"        return this->cache->{query['method_name']}({query['call_params']});"
#             end_line = "    }"

#             full_line = method_line + " \\\n" + return_line + " \\\n" + end_line
#             if i < len(queries) - 1:
#                 full_line += " \\"
#             lines.append(full_line)
#     else:
#         lines.append("// No queries defined")
#         lines.append("#define QUERY_FORWARDING_CACHE()")

#     lines.append("")
#     lines.append("#endif // CACHE_QUERY_FORWARDING_HPP")
#     return '\n'.join(lines)

# def generate_database_forwarding(queries):
#     """Generate forwarding implementations for Database"""
#     lines = []
#     lines.append("// Auto-generated file - DO NOT EDIT MANUALLY")
#     lines.append("#ifndef DATABASE_QUERY_FORWARDING_HPP")
#     lines.append("#define DATABASE_QUERY_FORWARDING_HPP")
#     lines.append("")

#     if queries:
#         lines.append("#define QUERY_FORWARDING_DATABASE() \\")
#         for i, query in enumerate(queries):
#             method_line = f"    {query['return_type']} Database::{query['method_name']}({query['full_params']}) {{"
#             return_line = f"        return this->database->{query['method_name']}({query['call_params']});"
#             end_line = "    }"

#             full_line = method_line + " \\\n" + return_line + " \\\n" + end_line
#             if i < len(queries) - 1:
#                 full_line += " \\"
#             lines.append(full_line)
#     else:
#         lines.append("// No queries defined")
#         lines.append("#define QUERY_FORWARDING_DATABASE()")

#     lines.append("")
#     lines.append("#endif // DATABASE_QUERY_FORWARDING_HPP")
#     return '\n'.join(lines)

# def generate_auth_config(queries):
#     """Generate authentication configuration"""
#     lines = []
#     lines.append("// Auto-generated file - DO NOT EDIT MANUALLY")
#     lines.append("#ifndef AUTH_CONFIG_HPP")
#     lines.append("#define AUTH_CONFIG_HPP")
#     lines.append("")
#     lines.append("#include <string>")
#     lines.append("#include <unordered_map>")
#     lines.append("#include <array>")
#     lines.append("")
#     lines.append("enum class AuthType { TOKEN };")
#     lines.append("")
#     lines.append("struct AuthConfig {")
#     lines.append("    AuthType type;")
#     lines.append("    std::string envVar;")
#     lines.append("};")
#     lines.append("")

#     if queries:
#         # Generate AutoToken array
#         auto_token_vars = []
#         for query in queries:
#             if query['auth_type'] == 'TOKEN' and query['key_behavior'] == 'AUTO':
#                 auto_token_vars.append(query['env_var_name'])

#         if auto_token_vars:
#             lines.append(f"static constexpr std::array<const char*, {len(auto_token_vars)}> AutoToken = {{")
#             for i, env_var in enumerate(auto_token_vars):
#                 line = f'    "{env_var}"'
#                 if i < len(auto_token_vars) - 1:
#                     line += ","
#                 lines.append(line)
#             lines.append("};")
#             lines.append("")

#         # Generate QUERY_AUTH_MAP
#         lines.append("static std::unordered_map<std::string, AuthConfig> QUERY_AUTH_MAP = {")
#         for i, query in enumerate(queries):
#             if query['auth_type']:  # Only add if auth_type is specified
#                 auth_type_enum = f"AuthType::{query['auth_type']}"
#                 env_var = query['env_var_name']
#                 line = f'    {{"{query["method_name"]}", {{{auth_type_enum}, "{env_var}"}}}}'
#                 if i < len(queries) - 1:
#                     line += ","
#                 lines.append(line)
#         lines.append("};")
#     else:
#         lines.append("// No queries defined")
#         lines.append("static constexpr std::array<const char*, 0> AutoToken = {};")
#         lines.append("static std::unordered_map<std::string, AuthConfig> QUERY_AUTH_MAP = {};")

#     lines.append("")
#     lines.append("#endif // AUTH_CONFIG_HPP")
#     return '\n'.join(lines)

# def generate_cmake_config(queries):
#     """Generate CMake configuration with compile definitions"""
#     lines = []
#     lines.append("# Auto-generated CMake configuration - DO NOT EDIT MANUALLY")
#     lines.append("")

#     if queries:
#         lines.append("# Query definitions found")
#         lines.append("add_compile_definitions(")
#         for query in queries:
#             lines.append(f"    QUERY_EXISTS_{query['method_name']}")
#         lines.append(")")
#         lines.append("")
#         lines.append("# Set individual query flags")
#         for query in queries:
#             lines.append(f"set(QUERY_EXISTS_{query['method_name']} TRUE)")
#     else:
#         lines.append("# No query definitions found")
#         lines.append("# target_compile_definitions will be empty")
#         lines.append("# No individual query flags to set")

#     lines.append("")
#     return '\n'.join(lines)

# def main():
#     parser = argparse.ArgumentParser(description='Generate query interfaces from definitions')
#     parser.add_argument('--definitions', required=True, help='Query definitions file')
#     parser.add_argument('--output-dir', required=True, help='Output directory for generated files')
#     args = parser.parse_args()

#     # Check if definitions file exists
#     if not os.path.exists(args.definitions):
#         print("Error: Definitions file not found: {args.definitions}")
#         return 1

#     # Parse definitions
#     queries = parse_definitions(args.definitions)

#     print("Found {len(queries)} queries to generate")

#     # Count auto tokens
#     auto_tokens = [q for q in queries if q['auth_type'] == 'TOKEN' and q['key_behavior'] == 'AUTO']
#     print("Found {len(auto_tokens)} auto-generated tokens")

#     # Create output directory
#     output_dir = Path(args.output_dir)
#     output_dir.mkdir(exist_ok=True)

#     # Generate all files (anche se queries è vuoto)
#     (output_dir / "Query_Definition.hpp").write_text(generate_query_definition(queries))
#     (output_dir / "Query_Override.hpp").write_text(generate_query_override(queries))
#     (output_dir / "Cache_Query_Forwarding.hpp").write_text(generate_cache_forwarding(queries))
#     (output_dir / "Database_Query_Forwarding.hpp").write_text(generate_database_forwarding(queries))
#     (output_dir / "AuthConfig.hpp").write_text(generate_auth_config(queries))
#     (output_dir / "Query_Config.cmake").write_text(generate_cmake_config(queries))

#     print("Files generated in: {output_dir}")
#     return 0

# if __name__ == '__main__':
#     exit(main())
