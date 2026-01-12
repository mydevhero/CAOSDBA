#!/usr/bin/env python3
"""
CAOS Query Generator - YAML/JSON Schema Edition
Generates C++ boilerplate code from query definitions.
"""

import argparse
import json
import logging
import sys
from pathlib import Path
from typing import Dict, List, Any, Optional, Tuple

import yaml
import jsonschema

# Configuration flags
USE_SCHEMA_VALIDATION = True  # Set to False to disable JSON schema validation
SCHEMA_PATH = Path(__file__).parent / "schemas" / "query-schema.json"

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)
logger = logging.getLogger(__name__)


class SchemaValidationError(Exception):
    """Raised when query definitions fail schema validation."""

    pass


class QueryDefinitionError(Exception):
    """Raised when query definitions contain logical errors."""

    pass


class QueryDefinitionParser:
    """Parses and validates query definitions from YAML files."""

    def __init__(self, schema_path: Optional[Path] = None):
        self.schema_path = schema_path or SCHEMA_PATH
        self.schema = None

        if USE_SCHEMA_VALIDATION and self.schema_path.exists():
            try:
                with open(self.schema_path, "r") as f:
                    self.schema = json.load(f)
                logger.debug(f"Loaded JSON schema from {self.schema_path}")
            except (json.JSONDecodeError, OSError) as e:
                logger.warning(f"Failed to load JSON schema: {e}")
                self.schema = None

    def _validate_schema(self, data: Dict) -> None:
        """Validate data against JSON schema if enabled."""
        if not USE_SCHEMA_VALIDATION or not self.schema:
            return

        try:
            jsonschema.validate(data, self.schema)
            logger.debug("Schema validation passed")
        except jsonschema.ValidationError as e:
            raise SchemaValidationError(f"Schema validation failed: {e.message}")

    def _parse_parameters(self, parameters: List[Dict]) -> Tuple[str, str]:
        """Convert YAML parameters to legacy format strings."""
        if not parameters:
            return "", ""

        param_strings = []
        param_names = []

        for param in parameters:
            param_type = param.get("type", "").strip()
            param_name = param.get("name", "").strip()

            if not param_type or not param_name:
                raise QueryDefinitionError(
                    f"Parameter missing type or name: {param}"
                )

            param_strings.append(f"{param_type} {param_name}")
            param_names.append(param_name)

        return ", ".join(param_strings), ", ".join(param_names)

    def _parse_authentication(
        self, auth_config: Optional[Dict]
    ) -> Tuple[str, str, str]:
        """Convert YAML authentication to legacy format."""
        if not auth_config:
            return "", "", ""

        auth_type = auth_config.get("type", "").strip().upper()
        env_var = auth_config.get("env_var", "").strip()
        required = auth_config.get("required", True)

        if not auth_type or auth_type == "NONE":
            return "", "", ""

        key_behavior = "REQUIRED" if required else "OPTIONAL"
        return auth_type, env_var, key_behavior

    def _infer_category_from_name(self, name: str) -> str:
        """Infer category from method name for backward compatibility."""
        if name.startswith("IQuery_Example_"):
            return "example"
        elif name.startswith("IQuery_Template_"):
            return "template"
        return "standard"

    def _parse_single_query(self, query_data: Dict) -> Dict[str, Any]:
        """Parse a single query definition from YAML to legacy format."""
        try:
            name = query_data.get("name", "").strip()
            if not name:
                raise QueryDefinitionError("Query missing 'name' field")

            # Determine category
            metadata = query_data.get("metadata", {})
            category = metadata.get("category", "").strip().lower()

            if not category:
                category = self._infer_category_from_name(name)

            # Validate category
            if category not in ["standard", "example", "template"]:
                raise QueryDefinitionError(
                    f"Invalid category '{category}' for query '{name}'. "
                    "Must be 'standard', 'example', or 'template'."
                )

            # Get enabled flag (default: true)
            enabled = query_data.get("enabled", True)

            # Parse parameters
            parameters = query_data.get("parameters", [])
            full_params, call_params = self._parse_parameters(parameters)

            # Parse authentication
            auth_config = query_data.get("authentication")
            auth_type, env_var_name, key_behavior = self._parse_authentication(
                auth_config
            )

            return {
                "name": name,
                "return_type": query_data.get("return_type", "").strip(),
                "full_params": full_params,
                "call_params": call_params,
                "auth_type": auth_type,
                "env_var_name": env_var_name,
                "key_behavior": key_behavior,
                "enabled": enabled,
                "category": category,
                "original_data": query_data,  # Keep for error reporting
            }

        except KeyError as e:
            raise QueryDefinitionError(f"Missing required field in query: {e}")

    def parse_file(self, file_path: Path) -> List[Dict[str, Any]]:
        """Parse and validate query definitions from YAML file."""
        logger.info(f"Parsing definitions from: {file_path}")

        if not file_path.exists():
            raise FileNotFoundError(f"Definitions file not found: {file_path}")

        # Load YAML
        try:
            with open(file_path, "r") as f:
                data = yaml.safe_load(f)
        except yaml.YAMLError as e:
            raise QueryDefinitionError(f"Invalid YAML format: {e}")

        if not isinstance(data, dict):
            raise QueryDefinitionError("YAML root must be a dictionary")

        # Validate against schema
        self._validate_schema(data)

        # Extract queries
        queries_data = data.get("queries", [])
        if not isinstance(queries_data, list):
            raise QueryDefinitionError("'queries' must be a list")

        parsed_queries = []
        for idx, query_data in enumerate(queries_data):
            try:
                parsed_query = self._parse_single_query(query_data)
                parsed_queries.append(parsed_query)
                logger.debug(f"Parsed query: {parsed_query['name']}")
            except QueryDefinitionError as e:
                raise QueryDefinitionError(
                    f"Error in query at index {idx}: {e}"
                )

        logger.info(f"Successfully parsed {len(parsed_queries)} queries")
        return parsed_queries


def validate_query_enablement(
    query: Dict[str, Any], args: argparse.Namespace
) -> Tuple[bool, str]:
    """
    Determine if a query should be enabled based on its configuration and CLI flags.

    Returns:
        Tuple[bool, str]: (enabled, reason)
    """
    enabled = query.get("enabled", True)
    category = query.get("category", "standard")
    name = query["name"]

    # Check for disabled queries with CLI flags present
    if not enabled:
        if category == "example" and args.caos_example_query:
            raise QueryDefinitionError(
                f"Query '{name}' is disabled (enabled: false) but "
                "--caos-example-query flag is present. "
                "Either remove the flag or enable the query."
            )
        elif category == "template" and args.caos_template_query:
            raise QueryDefinitionError(
                f"Query '{name}' is disabled (enabled: false) but "
                "--caos-template-query flag is present. "
                "Either remove the flag or enable the query."
            )
        return False, "query disabled"

    # Evaluate based on category
    if category == "example":
        if args.caos_example_query:
            return True, "example query enabled via flag"
        return False, "example query requires --caos-example-query"

    elif category == "template":
        if args.caos_template_query:
            return True, "template query enabled via flag"
        return False, "template query requires --caos-template-query"

    elif category == "standard":
        return True, "standard query always enabled"

    else:
        # This should not happen due to schema validation
        return False, f"unknown category '{category}'"


def convert_to_legacy_format(queries: List[Dict[str, Any]]) -> List[Dict[str, str]]:
    """Convert enriched query format to legacy 7-field format for generators."""
    legacy_queries = []

    for query in queries:
        legacy_queries.append({
            "return_type": query["return_type"],
            "method_name": query["name"],
            "full_params": query["full_params"],
            "call_params": query["call_params"],
            "auth_type": query["auth_type"],
            "env_var_name": query["env_var_name"],
            "key_behavior": query["key_behavior"],
        })

    return legacy_queries


def generate_query_definition(queries):
    """Generate macro for pure virtual definitions in IQuery."""
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
    return "\n".join(lines)


def generate_query_override(queries):
    """Generate macro for override declarations."""
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
    return "\n".join(lines)


def generate_cache_forwarding(queries):
    """Generate forwarding implementations for Cache."""
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
    return "\n".join(lines)


def generate_database_forwarding(queries):
    """Generate forwarding implementations for Database."""
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
    return "\n".join(lines)


def generate_auth_config(queries):
    """Generate authentication configuration."""
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
            if query["auth_type"] == "TOKEN" and query["key_behavior"] == "AUTO":
                auto_token_vars.append(query["env_var_name"])

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
            if query["auth_type"]:  # Only add if auth_type is specified
                auth_type_enum = f"AuthType::{query['auth_type']}"
                env_var = query["env_var_name"]
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
    return "\n".join(lines)


def generate_cmake_config(queries):
    """Generate CMake configuration with compile definitions."""
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
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Generate query interfaces from YAML definitions"
    )
    parser.add_argument(
        "--definitions",
        required=True,
        help="Query definitions YAML file",
    )
    parser.add_argument(
        "--output-dir",
        required=True,
        help="Output directory for generated files",
    )
    parser.add_argument(
        "--caos-example-query",
        action="store_true",
        default=False,
        help="Enable IQuery_Example_echoString and other example queries",
    )
    parser.add_argument(
        "--caos-template-query",
        action="store_true",
        default=False,
        help="Enable IQuery_Template_echoString and other template queries",
    )
    parser.add_argument(
        "--verbose", "-v", action="store_true", help="Enable verbose debug logging"
    )
    parser.add_argument(
        "--no-schema-validation",
        action="store_true",
        help="Disable JSON schema validation",
    )

    args = parser.parse_args()

    # Configure logging
    if args.verbose:
        logger.setLevel(logging.DEBUG)
        logger.debug("Verbose logging enabled")

    # Override schema validation if requested
    global USE_SCHEMA_VALIDATION
    if args.no_schema_validation:
        USE_SCHEMA_VALIDATION = False
        logger.info("JSON schema validation disabled")

    logger.info("=== CAOS Query Generator (YAML Edition) ===")

    try:
        # Parse query definitions
        parser = QueryDefinitionParser()
        all_queries = parser.parse_file(Path(args.definitions))

        # Filter queries based on enablement rules
        enabled_queries_data = []
        enabled_legacy_queries = []

        for query in all_queries:
            try:
                enabled, reason = validate_query_enablement(query, args)
                if enabled:
                    enabled_queries_data.append(query)
                    logger.info(f"✅ Enabling {query['name']} ({reason})")
                else:
                    logger.info(f"❌ Skipping {query['name']} ({reason})")
            except QueryDefinitionError as e:
                logger.error(f"🚨 Error validating query {query['name']}: {e}")
                return 1

        # Convert to legacy format for existing generators
        enabled_legacy_queries = convert_to_legacy_format(enabled_queries_data)

        logger.info(f"Total queries found: {len(all_queries)}")
        logger.info(f"Queries enabled: {len(enabled_legacy_queries)}")

        # Count auto tokens
        auto_tokens = [
            q
            for q in enabled_legacy_queries
            if q["auth_type"] == "TOKEN" and q["key_behavior"] == "AUTO"
        ]
        logger.info(f"Found {len(auto_tokens)} auto-generated tokens")

        # Create output directory
        output_dir = Path(args.output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
        logger.info(f"Output directory: {output_dir.absolute()}")

        # Generate all files
        logger.info("Generating files...")

        (output_dir / "Query_Definition.hpp").write_text(
            generate_query_definition(enabled_legacy_queries)
        )
        logger.info("✓ Generated: Query_Definition.hpp")

        (output_dir / "Query_Override.hpp").write_text(
            generate_query_override(enabled_legacy_queries)
        )
        logger.info("✓ Generated: Query_Override.hpp")

        (output_dir / "Cache_Query_Forwarding.hpp").write_text(
            generate_cache_forwarding(enabled_legacy_queries)
        )
        logger.info("✓ Generated: Cache_Query_Forwarding.hpp")

        (output_dir / "Database_Query_Forwarding.hpp").write_text(
            generate_database_forwarding(enabled_legacy_queries)
        )
        logger.info("✓ Generated: Database_Query_Forwarding.hpp")

        (output_dir / "AuthConfig.hpp").write_text(
            generate_auth_config(enabled_legacy_queries)
        )
        logger.info("✓ Generated: AuthConfig.hpp")

        (output_dir / "Query_Config.cmake").write_text(
            generate_cmake_config(enabled_legacy_queries)
        )
        logger.info("✓ Generated: Query_Config.cmake")

        logger.info("✅ Code generation completed successfully")
        return 0

    except FileNotFoundError as e:
        logger.error(f"File error: {e}")
        return 1
    except SchemaValidationError as e:
        logger.error(f"Schema validation error: {e}")
        return 1
    except QueryDefinitionError as e:
        logger.error(f"Query definition error: {e}")
        return 1
    except Exception as e:
        logger.error(f"Unexpected error: {e}", exc_info=args.verbose)
        return 1


if __name__ == "__main__":
    sys.exit(main())
