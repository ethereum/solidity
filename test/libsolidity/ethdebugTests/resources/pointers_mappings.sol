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
// .resources.pointers | keys: ["storage_43_10","storage_43_16","storage_43_20","storage_43_24","storage_43_29","storage_43_34","storage_43_42"]
// .resources.pointers.storage_43_10: {
//     "expect": [
//         "key"
//     ],
//     "for": {
//         "location": "storage",
//         "name": "balances",
//         "slot": {
//             "$keccak256": [
//                 {
//                     "$wordsized": "key"
//                 },
//                 {
//                     "$wordsized": "0x00"
//                 }
//             ]
//         }
//     }
// }
// .resources.pointers.storage_43_16: {
//     "expect": [
//         "key",
//         "key1"
//     ],
//     "for": {
//         "length": "0x01",
//         "location": "storage",
//         "name": "nested",
//         "slot": {
//             "$keccak256": [
//                 {
//                     "$wordsized": "key1"
//                 },
//                 {
//                     "$wordsized": {
//                         "$keccak256": [
//                             {
//                                 "$wordsized": "key"
//                             },
//                             {
//                                 "$wordsized": "0x01"
//                             }
//                         ]
//                     }
//                 }
//             ]
//         }
//     }
// }
// .resources.pointers.storage_43_20: {
//     "expect": [
//         "key"
//     ],
//     "for": {
//         "location": "storage",
//         "name": "byName",
//         "slot": {
//             "$keccak256": [
//                 "key",
//                 {
//                     "$wordsized": "0x02"
//                 }
//             ]
//         }
//     }
// }
// .resources.pointers.storage_43_24: {
//     "expect": [
//         "key"
//     ],
//     "for": {
//         "location": "storage",
//         "name": "byBytes",
//         "slot": {
//             "$keccak256": [
//                 "key",
//                 {
//                     "$wordsized": "0x03"
//                 }
//             ]
//         }
//     }
// }
// .resources.pointers.storage_43_29: {
//     "expect": [
//         "key"
//     ],
//     "for": {
//         "group": [
//             {
//                 "length": "0x01",
//                 "location": "storage",
//                 "name": "points-x",
//                 "slot": {
//                     "$keccak256": [
//                         {
//                             "$wordsized": "key"
//                         },
//                         {
//                             "$wordsized": "0x04"
//                         }
//                     ]
//                 }
//             },
//             {
//                 "length": "0x01",
//                 "location": "storage",
//                 "name": "points-y",
//                 "offset": "0x01",
//                 "slot": {
//                     "$keccak256": [
//                         {
//                             "$wordsized": "key"
//                         },
//                         {
//                             "$wordsized": "0x04"
//                         }
//                     ]
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.storage_43_34: {
//     "expect": [
//         "key"
//     ],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "lists-length",
//                 "slot": {
//                     "$keccak256": [
//                         {
//                             "$wordsized": "key"
//                         },
//                         {
//                             "$wordsized": "0x05"
//                         }
//                     ]
//                 }
//             },
//             {
//                 "define": {
//                     "lists-data": {
//                         "$keccak256": [
//                             {
//                                 "$wordsized": {
//                                     "$keccak256": [
//                                         {
//                                             "$wordsized": "key"
//                                         },
//                                         {
//                                             "$wordsized": "0x05"
//                                         }
//                                     ]
//                                 }
//                             }
//                         ]
//                     }
//                 },
//                 "in": {
//                     "list": {
//                         "count": {
//                             "$read": "lists-length"
//                         },
//                         "each": "lists-index",
//                         "is": {
//                             "location": "storage",
//                             "name": "lists-item",
//                             "slot": {
//                                 "$sum": [
//                                     "lists-data",
//                                     "lists-index"
//                                 ]
//                             }
//                         }
//                     }
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.storage_43_42: {
//     "expect": [
//         "key",
//         "key1",
//         "key2"
//     ],
//     "for": {
//         "length": "0x01",
//         "location": "storage",
//         "name": "deep",
//         "slot": {
//             "$keccak256": [
//                 {
//                     "$wordsized": "key2"
//                 },
//                 {
//                     "$wordsized": {
//                         "$keccak256": [
//                             {
//                                 "$wordsized": "key1"
//                             },
//                             {
//                                 "$wordsized": {
//                                     "$keccak256": [
//                                         {
//                                             "$wordsized": "key"
//                                         },
//                                         {
//                                             "$wordsized": "0x06"
//                                         }
//                                     ]
//                                 }
//                             }
//                         ]
//                     }
//                 }
//             ]
//         }
//     }
// }
