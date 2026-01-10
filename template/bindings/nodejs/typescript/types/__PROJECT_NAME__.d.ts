// ============================================================================
// CAOSDBA Node.js Native Bindings - TypeScript Definitions
// ============================================================================
// Project: __PROJECT_NAME__
// Backend: __CAOS_DB_BACKEND_LOWER__
// Build: __CAOS_BUILD_COUNT__
// Generated: __TIMESTAMP__
//
// This file provides TypeScript type definitions for the __PROJECT_NAME__
// native Node.js module. These definitions ensure type safety when calling
// CAOSDBA database operations from TypeScript/JavaScript.
//
// IMPORTANT FOR DEVELOPERS:
// 1. Update this file when adding new native functions in bindings.cpp
// 2. Keep the types synchronized with the C++ implementation
// 3. Run 'tsc --noEmit' to validate the types after changes
// ============================================================================

// Global namespace for TypeScript intellisense
declare global {
  namespace CaosModule {
    // Base types
    interface CallContextData {
      token?: string;
    }

    type CaosErrorType =
      | 'AUTH'
      | 'VALIDATION'
      | 'PARAMETER'
      | 'SYSTEM'
      | string;

    interface ErrorResult {
      success: false;
      error_type: CaosErrorType;
      error_message: string;
      data?: any;
    }

    interface SuccessResult<T = any> {
      success: true;
      data: T;
    }

    type CaosResult<T = any> = SuccessResult<T> | ErrorResult;

    interface BuildInfo {
      module: string;
      build: number;
      caos_initialized: boolean;
      node_version: string;
      napi_version: number;
      debug: boolean;
    }

    // Function declarations
    declare function getBuildInfo(): BuildInfo;

    /**
     * Echo string query - template example
     *
     * This function demonstrates the standard query pattern.
     * It takes a call context and a string, returns the same string or an error.
     *
     * @param callContext - Call context with optional authentication token
     * @param input - String to echo
     * @returns Result with echoed string or error
     *
     * C++ equivalent: IQuery_Template_echoString in bindings.cpp
     */
    declare function IQuery_Template_echoString(
      callContext: CallContextData,
      input: string
    ): CaosResult<string>;

    // ----------------------------------------------------------------------------
    // DECLARED QUERY FUNCTIONS
    //
    // TODO FOR DEVELOPERS:
    // 1. Add function declarations for ALL native functions exported in bindings.cpp
    // 2. Match parameter types exactly with C++ implementation
    // 3. Add JSDoc comments for each function
    // 4. Run 'tsc --noEmit' to validate after adding new functions
    // ----------------------------------------------------------------------------

    // ----------------------------------------------------------------------------
    // TODO: ADD MORE QUERY FUNCTION DECLARATIONS HERE
    //
    // Example template for new queries:
    //
    // /**
    //  * Query description
    //  *
    //  * Detailed description of what the query does.
    //  *
    //  * @param callContext - Call context with optional authentication token
    //  * @param param1 - Description of first parameter
    //  * @param param2 - Description of second parameter
    //  * @returns Result with return type or error
    //  *
    //  * C++ equivalent: IQuery_FunctionName in bindings.cpp
    //  */
    // declare function IQuery_FunctionName(
    //     callContext: CallContextData,
    //     param1: SomeType,
    //     param2: AnotherType
    // ): CaosResult<ReturnType>;
    // ----------------------------------------------------------------------------

    // Module interface
    interface CaosModule {
      getBuildInfo: typeof getBuildInfo;
      __version__: string;
      __auth_system__: string;
      __call_context_format__: string;
      __debug__: number;
    }
  }
}

// CommonJS export
export = CaosModule;

// Global namespace for non-module environments
export as namespace caos;

// Remember to:
// 1. Update this file when adding new native functions
// 2. Keep types synchronized with C++ implementation
// 3. Test with 'tsc --noEmit' and actual TypeScript code
// ----------------------------------------------------------------------------
