/*
 * test_main.cpp
 *
 *  Created on: 26 lug 2025
 *      Author: mrbi314
 */

#define CATCH_CONFIG_ENABLE_SOURCELINE_INFO
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_FAST_COMPILE

#include <iostream>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "libcaos.hpp"





// -------------------------------------------------------------------------------------------------
// Fixture (Setup/TearDown comune)
// -------------------------------------------------------------------------------------------------
struct TestFixture
{
  // private:

  public:
    TestFixture()
    {
        std::cout << "=== Setup fixture ===\n";
    }

    ~TestFixture()
    {
        std::cout << "=== Teardown fixture ===\n";
    }

    std::vector<std::string> valid_ipv4s =
    {
      "127.0.0.1",
      "192.168.1.1",
      "10.0.0.1"
    };

    std::vector<std::string> valid_ipv6s =
    {
      "::ffff:7f00:1",    // 127.0.0.1
      "::ffff:c0a8:101",  // 192.168.1.1
      "::ffff:a00:1",     // 10.0.0.1
      "::1"               // IPv6
    };

    std::vector<std::string> invalid_ips =
    {
      "256.0.0.1",
      "not.an.ip",
      "192.168.1.",
      "1234:5678:::9"     // IPv6 malformed
    };
};





// -------------------------------------------------------------------------------------------------
// Test Cases
// -------------------------------------------------------------------------------------------------
