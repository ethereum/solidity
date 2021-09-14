// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0.0;

// Intentionally badly wrapped and commented in weird places to get source locations inside code
// snippets in generated Yul. Also contains stuff that could break the assembly if not escaped properly.

contract C {} contract D /** @src 0:96:165  "contract D {..." */ {
    function f() /* @use-src 0:"input.sol", 1:"#utility.yul" @ast-id 15 */ public returns (string memory) { C c = new /// @src 0:149:156  "new C()"
        C(); c;
        string memory s = "/*"; s; return "/** @src 0:96:165  \"contract D {...\" */"
        ;
    }
}
