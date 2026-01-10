/**
 * __PROJECT_NAME__ - CAOSDBA Native Node.js Bindings
 *
 * @module __PROJECT_NAME__
 * @see ./types/__PROJECT_NAME__.d.ts for TypeScript definitions
 */
const native = require('./__PROJECT_NAME__.node');

module.exports = {
getBuildInfo: native.getBuildInfo,
IQuery_Template_echoString: native.IQuery_Template_echoString,

// Helper to check if native module loaded correctly
checkNative: function() {
return native.getBuildInfo !== undefined;
},

// Build information
__version__: '0.1.0',
__build__: __CAOS_BUILD_COUNT__,
__backend__: '__CAOS_DB_BACKEND_LOWER__',
__node_version__: '__NODE_MAJOR_VER__'
};
