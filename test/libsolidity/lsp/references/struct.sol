// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

struct MyStruct
//     ^ @MyStructDef
{
    uint value;
}

contract MyContract
{
    MyStruct thatStruct;
    //       ^ @thatStructDef

    function f(MyStruct memory _someStruct) public view returns (uint result)
    {
        MyStruct memory local = _someStruct;
        local.value = local.value + thatStruct.value;
        //                    ^ @thatStructUse (TODO)
        result = local.value;
        //       ^ @localUse
    }

    function g(uint lhs, uint rhs) public pure returns (uint result)
    {
        uint output = lhs + rhs;
        result = output;
    }
}

// ----
// -> textDocument/documentHighlight {
//     "position": @MyStructDef
// }
// <- [
//     {
//         "kind": 2,
//         "range": {
//             "end": {
//                 "character": 15,
//                 "line": 3
//             },
//             "start": {
//                 "character": 7,
//                 "line": 3
//             }
//         }
//     },
//     {
//         "kind": 2,
//         "range": {
//             "end": {
//                 "character": 12,
//                 "line": 11
//             },
//             "start": {
//                 "character": 4,
//                 "line": 11
//             }
//         }
//     },
//     {
//         "kind": 2,
//         "range": {
//             "end": {
//                 "character": 23,
//                 "line": 14
//             },
//             "start": {
//                 "character": 15,
//                 "line": 14
//             }
//         }
//     },
//     {
//         "kind": 2,
//         "range": {
//             "end": {
//                 "character": 16,
//                 "line": 16
//             },
//             "start": {
//                 "character": 8,
//                 "line": 16
//             }
//         }
//     }
// ]
// -> textDocument/documentHighlight {
//     "position": @thatStructDef
// }
// <- [
//     {
//         "kind": 3,
//         "range": {
//             "end": {
//                 "character": 23,
//                 "line": 11
//             },
//             "start": {
//                 "character": 13,
//                 "line": 11
//             }
//         }
//     },
//     {
//         "kind": 2,
//         "range": {
//             "end": {
//                 "character": 46,
//                 "line": 17
//             },
//             "start": {
//                 "character": 36,
//                 "line": 17
//             }
//         }
//     }
// ]
// -> textDocument/documentHighlight {
//     "position": @localUse
// }
// <- [
//     {
//         "kind": 3,
//         "range": {
//             "end": {
//                 "character": 29,
//                 "line": 16
//             },
//             "start": {
//                 "character": 24,
//                 "line": 16
//             }
//         }
//     },
//     {
//         "kind": 3,
//         "range": {
//             "end": {
//                 "character": 13,
//                 "line": 17
//             },
//             "start": {
//                 "character": 8,
//                 "line": 17
//             }
//         }
//     },
//     {
//         "kind": 2,
//         "range": {
//             "end": {
//                 "character": 27,
//                 "line": 17
//             },
//             "start": {
//                 "character": 22,
//                 "line": 17
//             }
//         }
//     },
//     {
//         "kind": 2,
//         "range": {
//             "end": {
//                 "character": 22,
//                 "line": 19
//             },
//             "start": {
//                 "character": 17,
//                 "line": 19
//             }
//         }
//     }
// ]
