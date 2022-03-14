// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

import {
    Lib as TheLib,
    RGBColor as ThatColor
} from "../goto/lib.sol";

contract C
{
    function one(uint lhs, uint rhs) public pure returns (uint result)
    {
        result = TheLib.add(lhs, rhs + 123);
        //       ^ @TheLibUse
    }

    function other() public pure returns (ThatColor memory output)
                                          // ^ @ThatColorRet
    {
        output.red = 50;
        output.green = output.red;
        output.blue = output.green;
    }
}
// ----
// lib: @diagnostics 2072
// -> textDocument/documentHighlight {
//     "position": @TheLibUse
// }
// <- [
//     {
//         "kind": 2,
//         "range": {
//             "end": {
//                 "character": 17,
//                 "line": 4
//             },
//             "start": {
//                 "character": 11,
//                 "line": 4
//             }
//         }
//     },
//     {
//         "kind": 2,
//         "range": {
//             "end": {
//                 "character": 23,
//                 "line": 12
//             },
//             "start": {
//                 "character": 17,
//                 "line": 12
//             }
//         }
//     }
// ]
// -> textDocument/documentHighlight {
//     "position": @ThatColorRet
// }
// <- [
//     {
//         "kind": 2,
//         "range": {
//             "end": {
//                 "character": 25,
//                 "line": 5
//             },
//             "start": {
//                 "character": 16,
//                 "line": 5
//             }
//         }
//     },
//     {
//         "kind": 2,
//         "range": {
//             "end": {
//                 "character": 51,
//                 "line": 16
//             },
//             "start": {
//                 "character": 42,
//                 "line": 16
//             }
//         }
//     }
// ]
