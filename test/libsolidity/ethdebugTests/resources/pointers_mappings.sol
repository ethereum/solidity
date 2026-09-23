struct Point { uint8 x; uint8 y; }

contract C {
    mapping(address => uint256) balances;
    mapping(address => mapping(uint256 => bool)) nested;
    mapping(string => uint256) byName;
    mapping(bytes => uint256) byBytes;
    mapping(uint256 => Point) points;
    mapping(uint256 => uint256[]) lists;
    mapping(bytes32 => mapping(bytes32 => mapping(bytes32 => uint8))) deep;
}
// ----
// .resources.types | keys: ["t_address","t_array$_t_uint256_$dyn_storage","t_bool","t_bytes32","t_bytes_memory_ptr","t_mapping$_t_address_$_t_mapping$_t_uint256_$_t_bool_$_$","t_mapping$_t_address_$_t_uint256_$","t_mapping$_t_bytes32_$_t_mapping$_t_bytes32_$_t_mapping$_t_bytes32_$_t_uint8_$_$_$","t_mapping$_t_bytes32_$_t_mapping$_t_bytes32_$_t_uint8_$_$","t_mapping$_t_bytes32_$_t_uint8_$","t_mapping$_t_bytes_memory_ptr_$_t_uint256_$","t_mapping$_t_string_memory_ptr_$_t_uint256_$","t_mapping$_t_uint256_$_t_array$_t_uint256_$dyn_storage_$","t_mapping$_t_uint256_$_t_bool_$","t_mapping$_t_uint256_$_t_struct$_Point_$6_storage_$","t_string_memory_ptr","t_struct$_Point_$6_storage","t_uint256","t_uint8"]
// .resources.pointers | keys: ["t_array$_t_uint256_$dyn_storage","t_mapping$_t_address_$_t_mapping$_t_uint256_$_t_bool_$_$","t_mapping$_t_address_$_t_uint256_$","t_mapping$_t_bytes32_$_t_mapping$_t_bytes32_$_t_mapping$_t_bytes32_$_t_uint8_$_$_$","t_mapping$_t_bytes32_$_t_mapping$_t_bytes32_$_t_uint8_$_$","t_mapping$_t_bytes32_$_t_uint8_$","t_mapping$_t_bytes_memory_ptr_$_t_uint256_$","t_mapping$_t_string_memory_ptr_$_t_uint256_$","t_mapping$_t_uint256_$_t_array$_t_uint256_$dyn_storage_$","t_mapping$_t_uint256_$_t_bool_$","t_mapping$_t_uint256_$_t_struct$_Point_$6_storage_$","t_struct$_Point_$6_storage"]
// .resources.pointers.t_array$_t_uint256_$dyn_storage: {
//     "expect": [
//         "slot"
//     ],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "length",
//                 "slot": "slot"
//             },
//             {
//                 "define": {
//                     "data": {
//                         "$keccak256": [
//                             {
//                                 "$wordsized": "slot"
//                             }
//                         ]
//                     }
//                 },
//                 "in": {
//                     "list": {
//                         "count": {
//                             "$read": "length"
//                         },
//                         "each": "index",
//                         "is": {
//                             "location": "storage",
//                             "name": "item",
//                             "slot": {
//                                 "$sum": [
//                                     "data",
//                                     "index"
//                                 ]
//                             }
//                         }
//                     }
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.t_mapping$_t_address_$_t_mapping$_t_uint256_$_t_bool_$_$: {
//     "expect": [
//         "slot",
//         "key"
//     ],
//     "for": {
//         "location": "storage",
//         "name": "value",
//         "slot": {
//             "$keccak256": [
//                 {
//                     "$wordsized": "key"
//                 },
//                 {
//                     "$wordsized": "slot"
//                 }
//             ]
//         }
//     }
// }
// .resources.pointers.t_mapping$_t_address_$_t_uint256_$: {
//     "expect": [
//         "slot",
//         "key"
//     ],
//     "for": {
//         "location": "storage",
//         "name": "value",
//         "slot": {
//             "$keccak256": [
//                 {
//                     "$wordsized": "key"
//                 },
//                 {
//                     "$wordsized": "slot"
//                 }
//             ]
//         }
//     }
// }
// .resources.pointers.t_mapping$_t_bytes32_$_t_mapping$_t_bytes32_$_t_mapping$_t_bytes32_$_t_uint8_$_$_$: {
//     "expect": [
//         "slot",
//         "key"
//     ],
//     "for": {
//         "location": "storage",
//         "name": "value",
//         "slot": {
//             "$keccak256": [
//                 {
//                     "$wordsized": "key"
//                 },
//                 {
//                     "$wordsized": "slot"
//                 }
//             ]
//         }
//     }
// }
// .resources.pointers.t_mapping$_t_bytes32_$_t_mapping$_t_bytes32_$_t_uint8_$_$: {
//     "expect": [
//         "slot",
//         "key"
//     ],
//     "for": {
//         "location": "storage",
//         "name": "value",
//         "slot": {
//             "$keccak256": [
//                 {
//                     "$wordsized": "key"
//                 },
//                 {
//                     "$wordsized": "slot"
//                 }
//             ]
//         }
//     }
// }
// .resources.pointers.t_mapping$_t_bytes32_$_t_uint8_$: {
//     "expect": [
//         "slot",
//         "key"
//     ],
//     "for": {
//         "length": "0x01",
//         "location": "storage",
//         "name": "value",
//         "offset": "0x1f",
//         "slot": {
//             "$keccak256": [
//                 {
//                     "$wordsized": "key"
//                 },
//                 {
//                     "$wordsized": "slot"
//                 }
//             ]
//         }
//     }
// }
// .resources.pointers.t_mapping$_t_bytes_memory_ptr_$_t_uint256_$: {
//     "expect": [
//         "slot",
//         "key"
//     ],
//     "for": {
//         "location": "storage",
//         "name": "value",
//         "slot": {
//             "$keccak256": [
//                 "key",
//                 {
//                     "$wordsized": "slot"
//                 }
//             ]
//         }
//     }
// }
// .resources.pointers.t_mapping$_t_string_memory_ptr_$_t_uint256_$: {
//     "expect": [
//         "slot",
//         "key"
//     ],
//     "for": {
//         "location": "storage",
//         "name": "value",
//         "slot": {
//             "$keccak256": [
//                 "key",
//                 {
//                     "$wordsized": "slot"
//                 }
//             ]
//         }
//     }
// }
// .resources.pointers.t_mapping$_t_uint256_$_t_array$_t_uint256_$dyn_storage_$: {
//     "expect": [
//         "slot",
//         "key"
//     ],
//     "for": {
//         "define": {
//             "slot": {
//                 "$keccak256": [
//                     {
//                         "$wordsized": "key"
//                     },
//                     {
//                         "$wordsized": "slot"
//                     }
//                 ]
//             }
//         },
//         "in": {
//             "template": "t_array$_t_uint256_$dyn_storage",
//             "yields": {
//                 "item": "value-item",
//                 "length": "value-length"
//             }
//         }
//     }
// }
// .resources.pointers.t_mapping$_t_uint256_$_t_bool_$: {
//     "expect": [
//         "slot",
//         "key"
//     ],
//     "for": {
//         "length": "0x01",
//         "location": "storage",
//         "name": "value",
//         "offset": "0x1f",
//         "slot": {
//             "$keccak256": [
//                 {
//                     "$wordsized": "key"
//                 },
//                 {
//                     "$wordsized": "slot"
//                 }
//             ]
//         }
//     }
// }
// .resources.pointers.t_mapping$_t_uint256_$_t_struct$_Point_$6_storage_$: {
//     "expect": [
//         "slot",
//         "key"
//     ],
//     "for": {
//         "define": {
//             "slot": {
//                 "$keccak256": [
//                     {
//                         "$wordsized": "key"
//                     },
//                     {
//                         "$wordsized": "slot"
//                     }
//                 ]
//             }
//         },
//         "in": {
//             "template": "t_struct$_Point_$6_storage",
//             "yields": {
//                 "x": "value-x",
//                 "y": "value-y"
//             }
//         }
//     }
// }
// .resources.pointers.t_struct$_Point_$6_storage: {
//     "expect": [
//         "slot"
//     ],
//     "for": {
//         "group": [
//             {
//                 "length": "0x01",
//                 "location": "storage",
//                 "name": "x",
//                 "offset": "0x1f",
//                 "slot": "slot"
//             },
//             {
//                 "length": "0x01",
//                 "location": "storage",
//                 "name": "y",
//                 "offset": "0x1e",
//                 "slot": "slot"
//             }
//         ]
//     }
// }
