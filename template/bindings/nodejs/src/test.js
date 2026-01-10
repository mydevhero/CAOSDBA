"use strict";
// make sure to have typescript installed: npm install -g typescript
Object.defineProperty(exports, "__esModule", { value: true });
// test.ts
const caos = require("my_app");
const callContext = { token: "ARBJi7cJuOYPXmFPPLVWsGrXmD4SU3LW" };
const result = caos.IQuery_Template_echoString(callContext, 'Hello TS');
console.log(result);
