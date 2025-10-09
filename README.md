# CAOS Framework

<div align="center">

**Cache App On Steroids - High-Performance C++ Backend with Built-in Caching**

</div>

**CAOS** (Cache App On Steroids) it's a high-performance **C++ Architectural Framework** designed for backends where **caching is not an optimization, but a foundational component**. It's built for speed, consistency, and scalability from the ground up.

---

## 🚀 Performance by Architectural Design

### **Data Access & Caching**
* **Cache-First Architecture:** Every critical request is directed to **Redis** first, ensuring **sub-millisecond response times**.
* **Smart Caching:** Configurable **cache invalidation strategies** and distributed caching support to guarantee data freshness and consistency.
* **Unified Access:** A consistent and **automated API** for interacting with all persistence layers.

### **Connection Infrastructure (Pool Management)**
* **Automatic Connection Pooling:** The framework intelligently manages a **pool of connections** (to Redis and the databases) to avoid resource saturation and the overhead of creation/destruction.
* **Controlled Scalability:** Allows configuration of **minimum and maximum** connection limits, enabling CAOS to optimize resource utilization elastically.

### **Multi-Database Support**
* **PostgreSQL:** Full ACID compliance for critical data.
* **MySQL/MariaDB:** Reliable, enterprise-grade relational storage.
* **Redis:** Blazing-fast in-memory caching layer.

### **Web & API Ready**
* **Built-in** HTTP server via **CrowCpp** for rapid service development.
* Out-of-the-box **REST API** endpoints, ideal for **Microservices** and **API Gateways**.

---

## ✨ Why Choose CAOS?

| Traditional Apps | CAOS-Based Apps |
| :--- | :--- |
| Cache is an **afterthought** | **Cache is the Foundation** |
| **Database-centric** architecture | **Cache-First Architecture** |
| **Slower** response times | **Sub-Millisecond Responses** |
| **Complex** caching setup | **Integrated** caching strategies |
| **Vertical Scaling** challenges | **Native Horizontal Scalability** |

---

## 🎯 Ideal For

* **High-Traffic Web Applications:** Efficient handling of thousands of concurrent users.
* **Real-Time Data Processing:** **Low-latency** data ingestion and processing.
* **Microservices Ecosystems:** Incredibly fast inter-service communication.
* **API Gateways:** Accelerating API responses and offloading DB load.
* **E-commerce/Gaming:** High-performance product catalogs and leaderboards.
* **Data-Intensive Backends:** Services requiring efficient processing of large datasets.

---

## 🛠️ Architecture & Technology

### **Architectural Benefits**
* **Reduced Database Load:** Smart caching can **cut direct DB queries by up to 90%**.
* **Improved Data Consistency:** Robust invalidation and synchronization mechanisms.
* **Developer Productivity:** Consistent and reusable data layer patterns.
* **Flexible Deployment:** Supports single instances and distributed clusters.

### **Supported Technologies**
| Category | Technologies |
| :--- | :--- |
| **Persistence** | PostgreSQL, MySQL, MariaDB |
| **Caching** | Redis |
| **Web/REST** | CrowCpp (HTTP/JSON) |
| **Language** | C++17+ |
| **Platform** | Linux |
| **Build System** | CMake |

---

## 💻 Requirements

### **System Requirements**
* **OS:** Linux (Ubuntu 18.04+, CentOS 8+, or equivalent)
* **RAM:** 4GB minimum, **8GB+ recommended** for optimal performance.
* **Storage:** 1GB available space.

### **Software Dependencies**
* **C++ Compiler:** g++ 13.0+ or clang++ 18.0+
* **CMake:** 3.15+
* **Redis:** 6.0+
* **Database:** PostgreSQL 12+ or MySQL 8.0+ or MariaDB 10.5+

### **Development Tools**
* **Build System:** CMake  
* **Package Manager:** Conan or vcpkg (optional, but recommended)

---

## 📦 Dependencies

CAOS uses:

* **CrowCpp:** [https://github.com/CrowCpp/Crow](https://github.com/CrowCpp/Crow)
* **hiredis:** `sudo apt install libhiredis-dev`
* **redis-plus-plus:** [https://github.com/sewenew/redis-plus-plus](https://github.com/sewenew/redis-plus-plus)
* **spdlog:** [https://github.com/gabime/spdlog](https://github.com/gabime/spdlog)
* **mariadb-connector-cpp:** [https://github.com/mariadb-corporation/mariadb-connector-cpp](https://github.com/mariadb-corporation/mariadb-connector-cpp)
* **libpqxx:** [https://github.com/jtv/libpqxx](https://github.com/jtv/libpqxx)
* **libmysqlcppconn:** `sudo apt install libmysqlcppconn-dev`
* **fmt:** [https://github.com/fmtlib/fmt](https://github.com/fmtlib/fmt)
* **cxxopts:** `sudo apt install libcxxopts-dev`

---

## 🧩 Query Architecture & Code Generation

The **CAOS Query System** introduces a powerful mechanism to define, generate, and implement data-access methods consistently across cache and database layers.

---

### 🔹 1. Query Definition System

All queries are defined in a **single central file**, `query_definitions.txt`, using a pipe-separated format:

```bash
# Example:
# std::optional<std::string> IQuery_Test_echoString(std::string str)

####################################################################################################
# return_type                   | method_name               | full_params           | call_params  #
####################################################################################################

std::optional<std::string>      | IQuery_Test_echoString    | std::string str       | str
```


This file acts as the **single source of truth** for the query system.  
The build process automatically generates all necessary boilerplate code from it.

---

### 🔹 2. Automatic Code Generation

The CAOS build system automatically creates the required interface and forwarding layers:

| Generated File | Description |
| :--- | :--- |
| `Query_Definition.hpp` | Contains pure virtual declarations for all queries in `IQuery`. |
| `Query_Override.hpp` | Declares overrides for intermediate layers (Cache and Database). |
| `Cache_Query_Forwarding.hpp` | Implements automatic forwarding for cache layers. |
| `Database_Query_Forwarding.hpp` | Implements automatic forwarding for database layers. |

> ⚠️ **Do not edit these files manually.**  
> They are regenerated at build time based on `query_definitions.txt`.

---

### 🔹 3. Architectural Layers

The query mechanism follows a structured layered model:

```
IQuery (interface)
↑
IRepository (base implementation)
↑
Cache / Database (forwarding layer)
↑
Redis / PostgreSQL / MySQL / MariaDB (concrete backends)
```


Each layer has a well-defined responsibility, ensuring consistency and separation of concerns.

---

### 🔹 4. Adding New Queries

To introduce a new query method:

1. Add a line to `query_definitions.txt`
2. Rebuild the project — the build system generates all virtual declarations and forwarding code.
3. Implement the actual logic in the proper backend:
* **Cache Backend:**  
  `libcaos/Middleware/Repository/Cache/<Backend>/Query.hpp`
* **Database Backend:**  
  `libcaos/Middleware/Repository/Database/<Backend>/Query.hpp`

---

## ⚙️ Implementation Details & Accessors

### 🔸 1. Cache Layer Implementation

When implementing a query in a cache backend (e.g., Redis), you can access:

| Accessor | Description |
| :--- | :--- |
| `this->redis` | Provides direct access to the cache client, supporting all hiredis and redis-plus-plus operations. |
| `this->database` | Gives access to the corresponding database layer, enabling cache-aside patterns. |

**Typical location:**  
`libcaos/Middleware/Repository/Cache/Redis/Query.hpp`

---

### 🔸 2. Database Layer Implementation

Database queries handle direct communication with the SQL backend.

| Accessor | Description |
| :--- | :--- |
| `this->database` | A connection pool accessor. Use `acquire()` to retrieve a connection object for executing SQL commands. |

**File locations:**
* `libcaos/Middleware/Repository/Database/PostgreSQL/Query.hpp`
* `libcaos/Middleware/Repository/Database/MySQL/Query.hpp`
* `libcaos/Middleware/Repository/Database/MariaDB/Query.hpp`

---

### 🔸 3. Manual Implementations

While CAOS auto-generates all structural boilerplate, **backend-specific query logic** must be implemented manually within each backend’s `Query.hpp` file.

These implementations define how each backend retrieves, caches, or updates data.

---

## 💡 Example Workflow

### Auto-Generated Components
* `IQuery::IQuery_Test_echoString` (pure virtual)
* Forwarding declarations in intermediate layers
* Implementations in Redis/Database forwarding headers

### Manual Implementations
* Cache logic → `Cache/Redis/Query.hpp`
* Database logic → `Database/PostgreSQL/Query.hpp` (or other DB backend)
