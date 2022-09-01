// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

contract C
{
    int public renameMe;
    //         ^^^^^^^^ @VariableInDefinition
    //         ^ @CursorOnVariableDefinition

    function foo() public returns(int)
    {
        renameMe = 1;
//      ^^^^^^^^ @VariableInFunction
//             ^ @CursorOnVariableInFunction
        return this.renameMe();
//                  ^^^^^^^^ @VariableInGetter
//                     ^ @CursorOnVariableInGetter
    }
}

function freeFunction(C _contract) view returns(int)
{
    return _contract.renameMe();
    //               ^^^^^^^^ @VariableInFreeFunction
    //                 ^ @CursorOnVariableInFreeFunction
}

// ----
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorOnVariableInFunction
// }
// <- {
//     "changes": {
//         "rename/variable.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInFreeFunction
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInGetter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInFunction
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInDefinition
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorOnVariableDefinition
// }
// <- {
//     "changes": {
//         "rename/variable.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInFreeFunction
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInGetter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInFunction
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInDefinition
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorOnVariableInGetter
// }
// <- {
//     "changes": {
//         "rename/variable.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInFreeFunction
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInGetter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInFunction
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInDefinition
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorOnVariableInFreeFunction
// }
// <- {
//     "changes": {
//         "rename/variable.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInFreeFunction
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInGetter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInFunction
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @VariableInDefinition
//             }
//         ]
//     }
// }
