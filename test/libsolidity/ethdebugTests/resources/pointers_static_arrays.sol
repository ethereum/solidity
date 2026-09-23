struct Point { uint8 x; uint8 y; }

contract C {
    uint16[8] packed;
    uint256[3] words;
    uint128[3] halves;
    bytes32[2][2] grid;
    Point[2] points;
}
// ----
// .resources.types | keys: ["t_array$_t_array$_t_bytes32_$2_storage_$2_storage","t_array$_t_bytes32_$2_storage","t_array$_t_struct$_Point_$6_storage_$2_storage","t_array$_t_uint128_$3_storage","t_array$_t_uint16_$8_storage","t_array$_t_uint256_$3_storage","t_bytes32","t_struct$_Point_$6_storage","t_uint128","t_uint16","t_uint256","t_uint8"]
// .resources.pointers | keys: ["t_array$_t_array$_t_bytes32_$2_storage_$2_storage","t_array$_t_bytes32_$2_storage","t_array$_t_struct$_Point_$6_storage_$2_storage","t_array$_t_uint128_$3_storage","t_array$_t_uint16_$8_storage","t_array$_t_uint256_$3_storage","t_struct$_Point_$6_storage"]
// .resources.pointers.t_array$_t_array$_t_bytes32_$2_storage_$2_storage: {
//     "expect": [
//         "slot"
//     ],
//     "for": {
//         "list": {
//             "count": "0x02",
//             "each": "index",
//             "is": {
//                 "define": {
//                     "slot": {
//                         "$sum": [
//                             "slot",
//                             {
//                                 "$product": [
//                                     "index",
//                                     "0x02"
//                                 ]
//                             }
//                         ]
//                     }
//                 },
//                 "in": {
//                     "template": "t_array$_t_bytes32_$2_storage",
//                     "yields": {
//                         "item": "item-item"
//                     }
//                 }
//             }
//         }
//     }
// }
// .resources.pointers.t_array$_t_bytes32_$2_storage: {
//     "expect": [
//         "slot"
//     ],
//     "for": {
//         "list": {
//             "count": "0x02",
//             "each": "index",
//             "is": {
//                 "location": "storage",
//                 "name": "item",
//                 "slot": {
//                     "$sum": [
//                         "slot",
//                         "index"
//                     ]
//                 }
//             }
//         }
//     }
// }
// .resources.pointers.t_array$_t_struct$_Point_$6_storage_$2_storage: {
//     "expect": [
//         "slot"
//     ],
//     "for": {
//         "list": {
//             "count": "0x02",
//             "each": "index",
//             "is": {
//                 "define": {
//                     "slot": {
//                         "$sum": [
//                             "slot",
//                             "index"
//                         ]
//                     }
//                 },
//                 "in": {
//                     "template": "t_struct$_Point_$6_storage",
//                     "yields": {
//                         "x": "item-x",
//                         "y": "item-y"
//                     }
//                 }
//             }
//         }
//     }
// }
// .resources.pointers.t_array$_t_uint128_$3_storage: {
//     "expect": [
//         "slot"
//     ],
//     "for": {
//         "list": {
//             "count": "0x03",
//             "each": "index",
//             "is": {
//                 "length": "0x10",
//                 "location": "storage",
//                 "name": "item",
//                 "offset": {
//                     "$difference": [
//                         "$wordsize",
//                         {
//                             "$product": [
//                                 {
//                                     "$sum": [
//                                         {
//                                             "$remainder": [
//                                                 "index",
//                                                 "0x02"
//                                             ]
//                                         },
//                                         "0x01"
//                                     ]
//                                 },
//                                 "0x10"
//                             ]
//                         }
//                     ]
//                 },
//                 "slot": {
//                     "$sum": [
//                         "slot",
//                         {
//                             "$quotient": [
//                                 "index",
//                                 "0x02"
//                             ]
//                         }
//                     ]
//                 }
//             }
//         }
//     }
// }
// .resources.pointers.t_array$_t_uint16_$8_storage: {
//     "expect": [
//         "slot"
//     ],
//     "for": {
//         "list": {
//             "count": "0x08",
//             "each": "index",
//             "is": {
//                 "length": "0x02",
//                 "location": "storage",
//                 "name": "item",
//                 "offset": {
//                     "$difference": [
//                         "$wordsize",
//                         {
//                             "$product": [
//                                 {
//                                     "$sum": [
//                                         {
//                                             "$remainder": [
//                                                 "index",
//                                                 "0x10"
//                                             ]
//                                         },
//                                         "0x01"
//                                     ]
//                                 },
//                                 "0x02"
//                             ]
//                         }
//                     ]
//                 },
//                 "slot": {
//                     "$sum": [
//                         "slot",
//                         {
//                             "$quotient": [
//                                 "index",
//                                 "0x10"
//                             ]
//                         }
//                     ]
//                 }
//             }
//         }
//     }
// }
// .resources.pointers.t_array$_t_uint256_$3_storage: {
//     "expect": [
//         "slot"
//     ],
//     "for": {
//         "list": {
//             "count": "0x03",
//             "each": "index",
//             "is": {
//                 "location": "storage",
//                 "name": "item",
//                 "slot": {
//                     "$sum": [
//                         "slot",
//                         "index"
//                     ]
//                 }
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
