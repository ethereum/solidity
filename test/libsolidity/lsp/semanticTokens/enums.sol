// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

enum Weather {
    Sunny,
    Cloudy,
    Rainy
}

enum Color {
    Red,
    Green,
    Blue
}

function getColorEnum() pure returns (Color result)
{
    result = Color.Red;
}
// ----
// -> textDocument/semanticTokens/full {
// }
// <- {
//     "data": [
//         1, 0, 24, 8, 0,
//         2, 5, 7, 2, 0,
//         1, 4, 5, 3, 0,
//         1, 4, 6, 3, 0,
//         1, 4, 5, 3, 0,
//         3, 5, 5, 2, 0,
//         1, 4, 3, 3, 0,
//         1, 4, 5, 3, 0,
//         1, 4, 4, 3, 0,
//         3, 9, 12, 5, 0,
//         0, 29, 5, 2, 0,
//         0, 6, 6, 19, 0,
//         2, 4, 6, 2, 0,
//         0, 9, 5, 2, 0,
//         0, 6, 3, 3, 0
//     ]
// }
