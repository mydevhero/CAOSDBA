// test-types.ts - Simple type validation
/// <reference path="./__PROJECT_NAME__.d.ts" />

// Test that types exist (compile-time only)
type TestCallContext = CaosModule.CallContextData;
type TestResult = CaosModule.CaosResult<string>;
type TestBuildInfo = CaosModule.BuildInfo;

// Test structures
const testContext: TestCallContext = { token: "test" };
const testResult: TestResult = { success: true, data: "test" };
const testBuildInfo: TestBuildInfo = {
    module: "test",
    build: __CAOS_BUILD_COUNT__,
    caos_initialized: true,
    node_version: "__NODE_MAJOR_VER__.0.0",
    napi_version: __NODE_NAPI_VERSION__,
    debug: false
};

// If this compiles, types are valid
console.log("TypeScript definitions are valid");
