// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

contract C
{
    bool public locked = false;
    int public calls = 0;
    int public totalSum = 0;

    function add(uint a, uint b) lock() monitor(a, b) public returns (uint result)
    {
        result = a + b;
    }

    modifier lock()
    {
        require(!locked);
        locked = true;
        _;
        locked = false;
    }

    modifier monitor(uint a, uint b)
    {
        calls++;
        totalSum = totalSum + a + b;
        //         ^^^^^^^^^^^^^^^^ @totalSumWarning
        //         ^^^^^^^^^^^^ @totalSumWarningSub
        _;
    }
}
// ----
// modifiers: @totalSumWarningSub 2271 @totalSumWarning 2271
// -> textDocument/semanticTokens/full {
// }
// <- {
//     "data": [
//         1, 0, 24, 8, 0,
//         2, 9, 1, 0, 0,
//         2, 4, 4, 11, 0,
//         0, 12, 6, 19, 0,
//         0, 9, 5, 11, 0,
//         1, 4, 3, 11, 0,
//         0, 11, 5, 19, 0,
//         0, 8, 1, 11, 0,
//         1, 4, 3, 11, 0,
//         0, 11, 8, 19, 0,
//         0, 11, 1, 11, 0,
//         2, 13, 3, 5, 0,
//         0, 4, 4, 11, 0,
//         0, 5, 1, 19, 0,
//         0, 3, 4, 11, 0,
//         0, 5, 1, 19, 0,
//         0, 40, 4, 11, 0,
//         0, 5, 6, 19, 0,
//         0, -42, 4, 19, 0,
//         0, 7, 7, 19, 0,
//         0, 8, 1, 19, 0,
//         0, 3, 1, 19, 0,
//         2, 8, 6, 19, 0,
//         0, 9, 1, 19, 0,
//         0, 4, 1, 19, 0,
//         3, 13, 4, 10, 0,
//         2, 8, 7, 19, 0,
//         0, 9, 6, 19, 0,
//         1, 8, 6, 19, 0,
//         0, 9, 4, 11, 0,
//         2, 8, 6, 19, 0,
//         0, 9, 5, 11, 0,
//         3, 13, 7, 10, 0,
//         0, 8, 4, 11, 0,
//         0, 5, 1, 19, 0,
//         0, 3, 4, 11, 0,
//         0, 5, 1, 19, 0,
//         2, 8, 5, 19, 0,
//         1, 8, 8, 19, 0,
//         0, 11, 8, 19, 0,
//         0, 11, 1, 19, 0,
//         0, 4, 1, 19, 0
//     ]
// }
