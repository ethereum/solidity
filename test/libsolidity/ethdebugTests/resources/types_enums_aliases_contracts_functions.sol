type Price is uint128;
enum Color { Red, Green, Blue }
interface I { function f() external; }
library L { function id(uint256 a) internal pure returns (uint256) { return a; } }

contract C {
    Color color;
    Price price;
    I other;
    C self;
    function (uint256) internal pure returns (uint256) internalFunction;
    function (uint256) external returns (bool) externalFunction;
    function g(uint256 a) public pure returns (uint256) { return L.id(a); }
}
// ----
// .resources.types | keys: ["t_bool","t_contract$_C_$64","t_contract$_I_$11","t_enum$_Color_$7","t_function_external_nonpayable$_t_uint256_$returns$_t_bool_$","t_function_internal_pure$_t_uint256_$returns$_t_uint256_$","t_uint128","t_uint256","t_userDefinedValueType$_Price_$3"]
// .resources.types.t_bool: {"kind":"bool"}
// .resources.types.t_contract$_C_$64: {
//     "definition": {
//         "location": {
//             "range": {
//                 "length": 287,
//                 "offset": 237
//             },
//             "source": {
//                 "id": 0
//             }
//         },
//         "name": "C"
//     },
//     "kind": "contract",
//     "payable": false
// }
// .resources.types.t_contract$_I_$11: {
//     "definition": {
//         "location": {
//             "range": {
//                 "length": 38,
//                 "offset": 114
//             },
//             "source": {
//                 "id": 0
//             }
//         },
//         "name": "I"
//     },
//     "interface": true,
//     "kind": "contract",
//     "payable": false
// }
// .resources.types.t_enum$_Color_$7: {
//     "definition": {
//         "location": {
//             "range": {
//                 "length": 31,
//                 "offset": 82
//             },
//             "source": {
//                 "id": 0
//             }
//         },
//         "name": "Color"
//     },
//     "kind": "enum",
//     "values": [
//         "Red",
//         "Green",
//         "Blue"
//     ]
// }
// .resources.types.t_function_external_nonpayable$_t_uint256_$returns$_t_bool_$: {
//     "contains": {
//         "parameters": {
//             "type": {
//                 "contains": [
//                     {
//                         "type": {
//                             "id": "t_uint256"
//                         }
//                     }
//                 ],
//                 "kind": "tuple"
//             }
//         },
//         "returns": {
//             "type": {
//                 "contains": [
//                     {
//                         "type": {
//                             "id": "t_bool"
//                         }
//                     }
//                 ],
//                 "kind": "tuple"
//             }
//         }
//     },
//     "external": true,
//     "kind": "function"
// }
// .resources.types.t_function_internal_pure$_t_uint256_$returns$_t_uint256_$: {
//     "contains": {
//         "parameters": {
//             "type": {
//                 "contains": [
//                     {
//                         "type": {
//                             "id": "t_uint256"
//                         }
//                     }
//                 ],
//                 "kind": "tuple"
//             }
//         },
//         "returns": {
//             "type": {
//                 "contains": [
//                     {
//                         "type": {
//                             "id": "t_uint256"
//                         }
//                     }
//                 ],
//                 "kind": "tuple"
//             }
//         }
//     },
//     "internal": true,
//     "kind": "function"
// }
// .resources.types.t_uint128: {"bits":128,"kind":"uint"}
// .resources.types.t_uint256: {"bits":256,"kind":"uint"}
// .resources.types.t_userDefinedValueType$_Price_$3: {
//     "contains": {
//         "type": {
//             "id": "t_uint128"
//         }
//     },
//     "definition": {
//         "location": {
//             "range": {
//                 "length": 22,
//                 "offset": 59
//             },
//             "source": {
//                 "id": 0
//             }
//         },
//         "name": "Price"
//     },
//     "kind": "alias"
// }
// .resources.pointers | keys: []
