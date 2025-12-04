/*
 * CAOS - <Cache App On Steroids>
 * Copyright (C) 2025  Alessandro Bianco
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * For support or inquiries, contact <mydevhero@gmail.com>
 */

/**
 * @file IQuery.hpp
 * @brief Base interface for all Repository Query methods in the CAOS Framework.
 *
 * ====================================================================
 * QUERY ARCHITECTURE & CODE GENERATION
 * ====================================================================
 *
 * 1.  QUERY DEFINITION SYSTEM:
 *
 * All queries are defined in a central definition file (`query_definitions.txt`)
 * using a simple pipe-separated format:
 *
 *   return_type | method_name | full_parameters | call_parameters
 *
 * Example:
 *   std::optional<std::string> | IQuery_Example_echoString | std::string str | str
 *
 * 2.  AUTOMATIC CODE GENERATION:
 *
 * The build system automatically generates all necessary boilerplate code:
 * - Virtual pure declarations in IQuery
 * - Override declarations in intermediate classes (Cache, Database)
 * - Forwarding implementations in Cache and Database layers
 *
 * 3.  IQuery CLASS (The Central Interface):
 *
 * The central interface class **IQuery** contains all query method signatures
 * as pure virtual functions. These are automatically generated from the
 * definitions file and included via the QUERY_DEFINITION() macro.
 *
 * 4.  IMPLEMENTATION LAYERS:
 *
 * The system follows a layered architecture:
 *
 *   IQuery (interface)
 *      ↑
 *   IRepository (base implementation)
 *      ↑
 *   Cache/Database (forwarding layer)
 *      ↑
 *   Redis/PostgreSQL/MySQL/MariaDB (concrete implementations)
 *
 * 5.  ADDING NEW QUERIES:
 *
 * To add a new query:
 * 1. Add a line to `query_definitions.txt` in the project root
 * 2. The build system will automatically regenerate all interfaces
 * 3. Implement the concrete logic in the appropriate backend:
 *    - Cache backends: `Middleware/Repository/Cache/<Backend>/Query.hpp`
 *    - Database backends: `Middleware/Repository/Database/<Backend>/Query.hpp`
 *
 * ====================================================================
 * IMPLEMENTATION DETAILS & ACCESSORS
 * ====================================================================
 *
 * 1.  CACHE LAYER IMPLEMENTATION:
 *
 * When implementing a query in a Cache backend (e.g., Redis), you have access to:
 * - The cache client: **`this->redis`** - Provides access to cache operations
 * - The database accessor: **`this->database`** - For cache-aside patterns
 *
 * 2.  DATABASE LAYER IMPLEMENTATION:
 *
 * When implementing a query in a Database backend, you have access to:
 * - The database pool: **`this->database`** - Use `acquire()` to get a connection
 * - The connection object for executing SQL queries
 *
 * 3.  GENERATED FILES:
 *
 * The following files are automatically generated and should NOT be edited manually:
 * - `Query_Definition.hpp` - Pure virtual declarations for IQuery
 * - `Query_Override.hpp` - Override declarations for intermediate classes
 * - `Cache_Query_Forwarding.hpp` - Forwarding implementations for Cache
 * - `Database_Query_Forwarding.hpp` - Forwarding implementations for Database
 *
 * 4.  MANUAL IMPLEMENTATIONS:
 *
 * Backend-specific implementations must be provided manually in the respective
 * backend Query.hpp files. The method signatures will match the generated
 * declarations exactly.
 *
 * ====================================================================
 * BACKEND SUPPORT
 * ====================================================================
 *
 * - **Cache Backends:** Redis (more can be added)
 * - **Database Backends:** PostgreSQL, MySQL, MariaDB (more can be added)
 *
 * Each backend must implement all query methods defined in the central
 * definitions file.
 */

#pragma once
#include "generated_queries/Query_Definition.hpp"

class IQuery
{
  public:

    QUERY_DEFINITION() /* <- from "generated_queries/Query_Definition.hpp" */

    // Manually insert your query definition here

    virtual ~IQuery() = default;
};
