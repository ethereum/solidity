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
// .resources.pointers | keys: ["storage_30_10","storage_30_14","storage_30_18","storage_30_24","storage_30_29"]
// .resources.pointers.storage_30_10: {
//     "expect": [],
//     "for": {
//         "list": {
//             "count": "0x08",
//             "each": "packed-index",
//             "is": {
//                 "length": "0x02",
//                 "location": "storage",
//                 "name": "packed-item",
//                 "offset": {
//                     "$product": [
//                         {
//                             "$remainder": [
//                                 "packed-index",
//                                 "0x10"
//                             ]
//                         },
//                         "0x02"
//                     ]
//                 },
//                 "slot": {
//                     "$sum": [
//                         "0x00",
//                         {
//                             "$quotient": [
//                                 "packed-index",
//                                 "0x10"
//                             ]
//                         }
//                     ]
//                 }
//             }
//         }
//     }
// }
// .resources.pointers.storage_30_14: {
//     "expect": [],
//     "for": {
//         "list": {
//             "count": "0x03",
//             "each": "words-index",
//             "is": {
//                 "location": "storage",
//                 "name": "words-item",
//                 "slot": {
//                     "$sum": [
//                         "0x01",
//                         "words-index"
//                     ]
//                 }
//             }
//         }
//     }
// }
// .resources.pointers.storage_30_18: {
//     "expect": [],
//     "for": {
//         "list": {
//             "count": "0x03",
//             "each": "halves-index",
//             "is": {
//                 "length": "0x10",
//                 "location": "storage",
//                 "name": "halves-item",
//                 "offset": {
//                     "$product": [
//                         {
//                             "$remainder": [
//                                 "halves-index",
//                                 "0x02"
//                             ]
//                         },
//                         "0x10"
//                     ]
//                 },
//                 "slot": {
//                     "$sum": [
//                         "0x04",
//                         {
//                             "$quotient": [
//                                 "halves-index",
//                                 "0x02"
//                             ]
//                         }
//                     ]
//                 }
//             }
//         }
//     }
// }
// .resources.pointers.storage_30_24: {
//     "expect": [],
//     "for": {
//         "list": {
//             "count": "0x02",
//             "each": "grid-index",
//             "is": {
//                 "list": {
//                     "count": "0x02",
//                     "each": "grid-item-index",
//                     "is": {
//                         "location": "storage",
//                         "name": "grid-item-item",
//                         "slot": {
//                             "$sum": [
//                                 {
//                                     "$sum": [
//                                         "0x06",
//                                         {
//                                             "$product": [
//                                                 "grid-index",
//                                                 "0x02"
//                                             ]
//                                         }
//                                     ]
//                                 },
//                                 "grid-item-index"
//                             ]
//                         }
//                     }
//                 }
//             }
//         }
//     }
// }
// .resources.pointers.storage_30_29: {
//     "expect": [],
//     "for": {
//         "list": {
//             "count": "0x02",
//             "each": "points-index",
//             "is": {
//                 "group": [
//                     {
//                         "length": "0x01",
//                         "location": "storage",
//                         "name": "points-item-x",
//                         "slot": {
//                             "$sum": [
//                                 "0x0a",
//                                 "points-index"
//                             ]
//                         }
//                     },
//                     {
//                         "length": "0x01",
//                         "location": "storage",
//                         "name": "points-item-y",
//                         "offset": "0x01",
//                         "slot": {
//                             "$sum": [
//                                 "0x0a",
//                                 "points-index"
//                             ]
//                         }
//                     }
//                 ]
//             }
//         }
//     }
// }
