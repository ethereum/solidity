// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

struct Tag
{
    uint id;
    string name;
}

struct RGBColor
{
    uint8 red;
    uint8 green;
    uint8 blue;
    Tag tag;
}

function memberAccess(RGBColor memory color) pure returns(uint)
{
    return color.red + color.green + color.blue;
}
// ----
// -> textDocument/semanticTokens/full {
// }
// <- {
//     "data": [
//         1, 0, 24, 8, 0,
//         4, 4, 4, 11, 0,
//         0, 5, 2, 19, 0,
//         1, 4, 6, 17, 0,
//         0, 7, 4, 19, 0,
//         5, 4, 5, 11, 0,
//         0, 6, 3, 19, 0,
//         1, 4, 5, 11, 0,
//         0, 6, 5, 19, 0,
//         1, 4, 5, 11, 0,
//         0, 6, 4, 19, 0,
//         1, 4, 3, 16, 0,
//         0, 4, 3, 19, 0,
//         3, 9, 12, 5, 0,
//         0, 13, 8, 16, 0,
//         0, 16, 5, 19, 0,
//         0, 20, 4, 11, 0,
//         2, 17, 3, 19, 0,
//         0, 12, 5, 19, 0,
//         0, 14, 4, 19, 0
//     ]
// }
