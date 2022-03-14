// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

import "../goto/lib.sol" as Imported;
//                          ^ @ImportedDef

contract C
{
    function one(uint lhs, uint rhs) public pure returns (uint result)
    {
        result = Imported.Lib.add(lhs, rhs + 123);
        //       ^ @ImportedUse
        //       ^^^^^^^^ @Imported1
        //       ^^^^^^^^^^^^ @ImportedLib1
    }
    function two(uint lhs, uint rhs) public pure returns (uint result)
    {
        result = one(Imported.Lib.add(lhs, rhs), rhs);
        //                    ^ @LibRef
        //           ^^^^^^^^ @Imported2
        //           ^^^^^^^^^^^^ @ImportedLib2
    }
}
// ----
// lib: @diagnostics 2072
// -> textDocument/documentHighlight {
//     "position": @ImportedDef
// }
// <- [
//     { "kind": 2, "range": @Imported1 },
//     { "kind": 2, "range": @Imported2 }
// ]
// -> textDocument/documentHighlight {
//     "position": @ImportedUse
// }
// <- [
//     { "kind": 2, "range": @Imported1 },
//     { "kind": 2, "range": @Imported2 }
// ]
// -> textDocument/documentHighlight {
//     "position": @LibRef
// }
// <- [
//     { "kind": 2, "range": @ImportedLib1 },
//     { "kind": 2, "range": @ImportedLib2 }
// ]
