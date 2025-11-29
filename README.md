<div>
<img src="https://raw.githubusercontent.com/mydevhero/CAOSDBA/main/docs/assets/caosdba.svg" alt="CAOSDBA" width="600"/>

<p><b>Cache App On Steroids - High-Performance C++ backend with built-in caching</b></p>
</div>

&nbsp;

## Why CAOSDBA?

**CAOSDBA** (**C**ache-**A**pp-**O**n-**S**teroids for **D**ata**b**ase **A**rchitects), also referred to as **CAOS** is a high-performance C++ framework that makes caching your **primary data layer**. Built for DBAs who need **predictable low-latency** and **zero-downtime scalability**. Designed for systems where caching isn't an optimization—it's the **foundational component**.

It integrates [**CrowCpp**](https://github.com/CrowCpp/Crow) as its backend engine and exposes C++ queries as **native PHP extensions**. Execute high-performance C++ code seamlessly through standard PHP function calls.

&nbsp;

## 👑 Designed for Database experts

**Database performance starts with proper query design**—something DBAs master daily. While backend developers excel at application logic, DBAs own the data layer expertise.

### **The reality CAOSDBA solves:**
- **DBAs** write optimal queries but can't always enforce their usage in application code
- **Developers** may lack deep SQL tuning expertise for complex data patterns
- **Performance gaps** emerge when query logic lives far from database expertise

### **The approach:**
- **Query ownership** stays with DBA teams where it belongs
- **C++ compiled queries** ensure optimal execution plans every time
- **CrowCpp or PHP simple interfaces** let developers call expert-level queries safely
- **No more "query degradation"** between DBA specs and implementation

&nbsp;

## ⚡ Performance by architectural design

### **Data access & caching:**
* **Cache-First architecture:** Every critical request is directed to **Redis** first, ensuring **sub-millisecond response times**.
* **Smart caching:** Configurable **cache invalidation strategies** and distributed caching support to guarantee data freshness and consistency.
* **Unified access:** A consistent and **automated API** for interacting with all persistence layers.

### **Connection infrastructure (Pool Management):**
* **Automatic connection pooling:** The framework intelligently manages a **pool of connections** (to Redis and the databases) to avoid resource saturation and the overhead of creation/destruction.
* **Controlled scalability:** Allows configuration of **minimum and maximum** connection limits, enabling CAOSDBA to optimize resource utilization elastically.

### **Multi-Database support:**
* **PostgreSQL:** Full ACID compliance for critical data.
* **MySQL/MariaDB:** Reliable, enterprise-grade relational storage.
* **Redis:** Blazing-fast in-memory caching layer.

### **Web & API ready:**
* **Built-in** HTTP server via **CrowCpp** for rapid service development
* Out-of-the-box **REST API** endpoints, ideal for **microservices** and **API Gateways**
* **Native PHP exports** - call C++ queries directly as PHP functions

&nbsp;

## 🧠 Architecture & Technology

### **Architectural benefits:**
* **Reduced Database Load:** Smart caching can **cut direct DB queries by up to 90%**.
* **Improved Data Consistency:** Robust invalidation and synchronization mechanisms.
* **Developer Productivity:** Consistent and reusable data layer patterns.
* **Flexible Deployment:** Supports single instances and distributed clusters.

&nbsp;

## 🎯 Ideal For

* **High-Traffic web applications:** Efficient handling of thousands of concurrent users.
* **Near Real-Time data processing:** **Low-latency** data ingestion and processing.
* **Microservices ecosystems:** Incredibly fast inter-service communication.
* **API Gateways:** Accelerating API responses and offloading DB load.
* **E-commerce/Gaming:** High-performance product catalogs and leaderboards.
* **Data-Intensive backends:** Services requiring efficient processing of large datasets like ERP, WMS or CRM

### **Supported Technologies:**
| Category | Technologies |
| :--- | :--- |
| **Persistence** | PostgreSQL, MySQL, MariaDB |
| **Caching** | Redis |
| **Web/REST** | CrowCpp (HTTP/JSON) - PHP native extension |
| **Language** | C++17+ |
| **Platform** | Linux |
| **Build System** | CMake |

&nbsp;

## 📦 Dependencies

CAOS uses:

* **CrowCpp**
* **hiredis**
* **redis-plus-plus**
* **spdlog**
* **mariadb-connector-cpp**
* **libpqxx**
* **libmysqlcppconn**
* **fmt**
* **cxxopts**
