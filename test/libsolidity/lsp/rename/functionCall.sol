// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

contract C
{
    function foo(int a, int b, int c) pure public returns(int)
    //                      ^ @ParameterB
    //               ^ @ParameterA
    //                             ^ @ParameterC
    {
        return a + b + c;
        //         ^ @ParameterBInFoo
        //     ^ @ParameterAInFoo
        //             ^ @ParameterCInFoo

    }

    function bar() public view
    {
        this.foo({c:1, b:2, a:3});
        //             ^ @ParameterBInCall
        //        ^ @ParameterCInCall
        //                  ^ @ParameterAInCall
    }
}
// ----
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @ParameterA
// }
// <- {
//     "changes": {
//         "rename/functionCall.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterAInCall
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterAInFoo
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterA
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @ParameterAInCall
// }
// <- {
//     "changes": {
//         "rename/functionCall.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterAInCall
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterAInFoo
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterA
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @ParameterAInFoo
// }
// <- {
//     "changes": {
//         "rename/functionCall.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterAInCall
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterAInFoo
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterA
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @ParameterC
// }
// <- {
//     "changes": {
//         "rename/functionCall.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterCInCall
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterCInFoo
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterC
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @ParameterCInCall
// }
// <- {
//     "changes": {
//         "rename/functionCall.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterCInCall
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterCInFoo
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterC
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @ParameterCInFoo
// }
// <- {
//     "changes": {
//         "rename/functionCall.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterCInCall
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterCInFoo
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterC
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @ParameterBInCall
// }
// <- {
//     "changes": {
//         "rename/functionCall.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterBInCall
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterBInFoo
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterB
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @ParameterBInFoo
// }
// <- {
//     "changes": {
//         "rename/functionCall.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterBInCall
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterBInFoo
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ParameterB
//             }
//         ]
//     }
// }
