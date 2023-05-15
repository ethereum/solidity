// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

library Lib
{
    function add(uint a, uint b) public pure returns (uint result)
    {
        result = a + b;
    }

    function warningWithUnused() public pure
    {
        uint unused;
//      ^^^^^^^^^^^ @unusedVariable
    }
}

contract Contract
{
    function doNothing() pure public returns (bool)
    {
        return true;
    }
}
// ----
// functions: @unusedVariable 2072
// -> textDocument/semanticTokens/full {
// }
// <- {
//     "data": [
//         1, 0, 24, 8, 0,
//         2, 8, 3, 0, 0,
//         2, 13, 3, 5, 0,
//         0, 4, 4, 11, 0,
//         0, 5, 1, 19, 0,
//         0, 3, 4, 11, 0,
//         0, 5, 1, 19, 0,
//         0, 24, 4, 11, 0,
//         0, 5, 6, 19, 0,
//         2, 8, 6, 19, 0,
//         0, 9, 1, 19, 0,
//         0, 4, 1, 19, 0,
//         3, 13, 17, 5, 0,
//         2, 8, 4, 11, 0,
//         0, 5, 6, 19, 0,
//         5, 9, 8, 0, 0,
//         2, 13, 9, 5, 0,
//         0, 33, 4, 11, 0,
//         2, 15, 4, 11, 0
//     ]
// }
