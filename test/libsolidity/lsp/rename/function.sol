// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

contract C
{
    function renameMe() public pure returns (int)
    //       ^^^^^^^^ @FunctionInDefinition
    //            ^ @CursorInDefinition
    {
        return 1;
    }

    function other() public view
    {
        renameMe();
//      ^^^^^^^^ @FunctionInFunctionSameContract
//         ^ @CursorInFunctionSameContract
        this.renameMe();
     //      ^^^^^^^^ @FunctionInFunctionSameContractExternal
         //         ^ @CursorInFunctionSameContractExternal
    }
}

contract Other
{
    C m_contract;

    function other() public view
    {
        m_contract.renameMe();
           //      ^^^^^^^^ @FunctionInFunctionOtherContract
        //         ^ @CursorInFunctionOtherContract
    }
}

function free() pure
{
    C local_contract;
    local_contract.renameMe();
           //      ^^^^^^^^ @FunctionInFreeFunction
        //         ^ @CursorInFreeFunction
}

// ----
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorInDefinition
// }
// <- {
//     "changes": {
//         "rename/function.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFreeFunction
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionOtherContract
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionSameContractExternal
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionSameContract
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInDefinition
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorInFunctionOtherContract
// }
// <- {
//     "changes": {
//         "rename/function.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFreeFunction
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionOtherContract
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionSameContractExternal
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionSameContract
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInDefinition
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorInFunctionSameContractExternal
// }
// <- {
//     "changes": {
//         "rename/function.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFreeFunction
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionOtherContract
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionSameContractExternal
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionSameContract
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInDefinition
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorInFunctionSameContract
// }
// <- {
//     "changes": {
//         "rename/function.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFreeFunction
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionOtherContract
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionSameContractExternal
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionSameContract
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInDefinition
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorInFreeFunction
// }
// <- {
//     "changes": {
//         "rename/function.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFreeFunction
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionOtherContract
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionSameContractExternal
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInFunctionSameContract
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @FunctionInDefinition
//             }
//         ]
//     }
// }
