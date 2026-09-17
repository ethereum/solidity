struct Point { uint8 x; uint8 y; bytes4 salt; }
struct Line { Point from; Point to; string label; }

contract C {
    Point point;
    Line line;
    mapping(address => uint256) balances;
    mapping(address => mapping(uint256 => Line)) lines;
    mapping(string => bool) named;
}
// ----
// .resources.types | keys: ["t_address","t_bool","t_bytes4","t_mapping$_t_address_$_t_mapping$_t_uint256_$_t_struct$_Line_$17_storage_$_$","t_mapping$_t_address_$_t_uint256_$","t_mapping$_t_string_memory_ptr_$_t_bool_$","t_mapping$_t_uint256_$_t_struct$_Line_$17_storage_$","t_string_memory_ptr","t_string_storage","t_struct$_Line_$17_storage","t_struct$_Point_$8_storage","t_uint256","t_uint8"]
// .resources.types.t_address: {"kind":"address","payable":false}
// .resources.types.t_bool: {"kind":"bool"}
// .resources.types.t_bytes4: {"kind":"bytes","size":4}
// .resources.types.t_mapping$_t_address_$_t_mapping$_t_uint256_$_t_struct$_Line_$17_storage_$_$: {
//     "contains": {
//         "key": {
//             "type": {
//                 "id": "t_address"
//             }
//         },
//         "value": {
//             "type": {
//                 "id": "t_mapping$_t_uint256_$_t_struct$_Line_$17_storage_$"
//             }
//         }
//     },
//     "kind": "mapping"
// }
// .resources.types.t_mapping$_t_address_$_t_uint256_$: {
//     "contains": {
//         "key": {
//             "type": {
//                 "id": "t_address"
//             }
//         },
//         "value": {
//             "type": {
//                 "id": "t_uint256"
//             }
//         }
//     },
//     "kind": "mapping"
// }
// .resources.types.t_mapping$_t_string_memory_ptr_$_t_bool_$: {
//     "contains": {
//         "key": {
//             "type": {
//                 "id": "t_string_memory_ptr"
//             }
//         },
//         "value": {
//             "type": {
//                 "id": "t_bool"
//             }
//         }
//     },
//     "kind": "mapping"
// }
// .resources.types.t_mapping$_t_uint256_$_t_struct$_Line_$17_storage_$: {
//     "contains": {
//         "key": {
//             "type": {
//                 "id": "t_uint256"
//             }
//         },
//         "value": {
//             "type": {
//                 "id": "t_struct$_Line_$17_storage"
//             }
//         }
//     },
//     "kind": "mapping"
// }
// .resources.types.t_string_memory_ptr: {"kind":"string"}
// .resources.types.t_string_storage: {"kind":"string"}
// .resources.types.t_struct$_Line_$17_storage: {
//     "contains": [
//         {
//             "name": "from",
//             "type": {
//                 "id": "t_struct$_Point_$8_storage"
//             }
//         },
//         {
//             "name": "to",
//             "type": {
//                 "id": "t_struct$_Point_$8_storage"
//             }
//         },
//         {
//             "name": "label",
//             "type": {
//                 "id": "t_string_storage"
//             }
//         }
//     ],
//     "definition": {
//         "location": {
//             "range": {
//                 "length": 51,
//                 "offset": 107
//             },
//             "source": {
//                 "id": 0
//             }
//         },
//         "name": "Line"
//     },
//     "kind": "struct"
// }
// .resources.types.t_struct$_Point_$8_storage: {
//     "contains": [
//         {
//             "name": "x",
//             "type": {
//                 "id": "t_uint8"
//             }
//         },
//         {
//             "name": "y",
//             "type": {
//                 "id": "t_uint8"
//             }
//         },
//         {
//             "name": "salt",
//             "type": {
//                 "id": "t_bytes4"
//             }
//         }
//     ],
//     "definition": {
//         "location": {
//             "range": {
//                 "length": 47,
//                 "offset": 59
//             },
//             "source": {
//                 "id": 0
//             }
//         },
//         "name": "Point"
//     },
//     "kind": "struct"
// }
// .resources.types.t_uint256: {"bits":256,"kind":"uint"}
// .resources.types.t_uint8: {"bits":8,"kind":"uint"}
// .resources.pointers | keys: ["storage_39_20","storage_39_23","storage_39_27","storage_39_34","storage_39_38"]
