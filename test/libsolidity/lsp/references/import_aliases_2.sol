// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

import {RGBColor as A, RGBColor as B, RGBColor as C} from "../goto/lib.sol";
//                  ^ @AliasA

contract MyContract
{
    function f(A memory a, B memory b) public pure returns (C memory c)
//             ^ @FuncParamTypeA
    {
        c.red = b.red + a.green + b.blue;
    }
}
// ----
// lib: @diagnostics 2072
// -> textDocument/documentHighlight {
//     "position": @FuncParamTypeA
// }
// <- [
//     { "kind": 2, "range": @AliasA },
//     { "kind": 2, "range": @FuncParamTypeA }
// ]
// -> textDocument/documentHighlight {
//     "position": @FuncParamTypeA
// }
// <- [
//     { "kind": 2, "range": @AliasA },
//     { "kind": 2, "range": @FuncParamTypeA }
// ]
