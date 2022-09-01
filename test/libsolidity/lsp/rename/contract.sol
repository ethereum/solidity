// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

contract ToRename
//       ^ @CursorOnContractDefinition
//       ^^^^^^^^ @ContractInDefinition
{
}

contract User
//       ^^^^ @UserContractInContractTest
{
    ToRename public publicVariable;
//  ^^^^^^^^ @ContractInPublicVariable
//         ^ @CursorOnPublicVariableType

    ToRename[10] previousContracts;
//  ^^^^^^^^ @ContractInArrayType
//   ^ @CursorOnArrayType

    mapping(int => ToRename) contractMapping;
    //             ^^^^^^^^ @ContractInMapping
    //              ^ @CursorOnMapping

    function getContract() public returns (ToRename)
                                  //       ^^^^^^^^ @ContractInReturnParameter
//                                            ^ @CursorOnReturnParameter
    {
        return new ToRename();
        //         ^^^^^^^^ @ContractInReturnExpression
//                    ^ @CursorOnReturnExpression
    }

    function setContract(ToRename _contract) public
    //                   ^^^^^^^^ @ContractInParameter
    //                        ^ @CursorOnParameter
    {
        publicVariable = _contract;
    }
}
// ----
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorOnContractDefinition
// }
// <- {
//     "changes": {
//         "rename/contract.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInParameter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInReturnExpression
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInReturnParameter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInMapping
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInArrayType
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInPublicVariable
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInDefinition
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorOnReturnParameter
// }
// <- {
//     "changes": {
//         "rename/contract.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInParameter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInReturnExpression
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInReturnParameter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInMapping
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInArrayType
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInPublicVariable
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInDefinition
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorOnReturnExpression
// }
// <- {
//     "changes": {
//         "rename/contract.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInParameter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInReturnExpression
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInReturnParameter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInMapping
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInArrayType
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInPublicVariable
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInDefinition
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorOnPublicVariableType
// }
// <- {
//     "changes": {
//         "rename/contract.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInParameter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInReturnExpression
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInReturnParameter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInMapping
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInArrayType
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInPublicVariable
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInDefinition
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorOnArrayType
// }
// <- {
//     "changes": {
//         "rename/contract.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInParameter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInReturnExpression
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInReturnParameter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInMapping
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInArrayType
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInPublicVariable
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInDefinition
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorOnMapping
// }
// <- {
//     "changes": {
//         "rename/contract.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInParameter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInReturnExpression
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInReturnParameter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInMapping
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInArrayType
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInPublicVariable
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInDefinition
//             }
//         ]
//     }
// }
// -> textDocument/rename {
//     "newName": "Renamed",
//     "position": @CursorOnParameter
// }
// <- {
//     "changes": {
//         "rename/contract.sol": [
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInParameter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInReturnExpression
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInReturnParameter
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInMapping
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInArrayType
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInPublicVariable
//             },
//             {
//                 "newText": "Renamed",
//                 "range": @ContractInDefinition
//             }
//         ]
//     }
// }
