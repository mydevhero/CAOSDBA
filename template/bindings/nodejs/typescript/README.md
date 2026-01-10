# TypeScript Support for CAOSDBA Node.js Bindings

This directory contains TypeScript template files for CAOSDBA Node.js native bindings.

## Files

### `types/__PROJECT_NAME__.d.ts`
The main TypeScript declaration file that provides type definitions for all native functions exported by the Node.js module. This file will be renamed to match your project name during initialization.

### `types/test-types.ts`
TypeScript test file that validates the type definitions compile correctly. Use this to ensure your TypeScript definitions match the actual native module exports.

### `tsconfig.json`
TypeScript configuration file optimized for Node.js native modules. Provides strict type checking and CommonJS compatibility.

### `.vscode/settings.json`
VS Code editor settings for TypeScript development.

### `.vscode/extensions.json`
Recommended VS Code extensions for TypeScript development.

## Usage

### After Project Initialization

When you run `caosdba.sh --init` for a Node.js binding project:

1. These files will be copied to `typescript/` in your project directory
2. All `__PROJECT_NAME__` placeholders will be replaced with your actual project name
3. The files will be ready for TypeScript development

### Updating Type Definitions

When you add new native functions in `src/bindings.cpp`:

1. Update the TypeScript definitions in `typescript/types/[your-project-name].d.ts`
2. Add function declarations matching the C++ function signatures
3. Run validation: `tsc --noEmit --project typescript/tsconfig.json`
4. Update `test-types.ts` if needed

### TypeScript Development Workflow

1. **Development**: Edit `.d.ts` files to match your native module
2. **Validation**: Run `tsc --noEmit` to check for type errors
3. **Testing**: Use `test-types.ts` to validate type compatibility
4. **Building**: The TypeScript files are included in the DEB package

## Integration with Build System

The CMake build system will:

1. Copy these TypeScript files to each Node.js version package directory
2. Include them in the DEB packages
3. Install them to `/usr/lib/x86_64-linux-gnu/nodejs/[project]/types/`
4. Configure `package.json` to point to the type definitions

## Notes for Developers

- Keep TypeScript definitions synchronized with C++ implementations
- Use JSDoc comments for better IDE support
- Test type definitions with actual TypeScript code
- Update this README if you modify the TypeScript support structure

## TypeScript Features Provided

- Complete type definitions for all native exports
- Discriminated union types for success/error results
- Type guards (`isSuccess`, `isError`)
- CallContext interface matching C++ structure
- BuildInfo interface for module metadata
- Module constants type definitions
- ES Modules and CommonJS compatibility
- VS Code integration settings

## Support

For issues with TypeScript definitions:
1. Check that function signatures match between C++ and TypeScript
2. Verify TypeScript compiler version compatibility
3. Ensure proper module resolution in your project
4. Test with the included `test-types.ts` file
