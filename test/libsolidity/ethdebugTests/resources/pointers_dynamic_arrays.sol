struct Point { uint8 x; uint8 y; }

contract C {
    uint256[] words;
    uint8[] packed;
    uint256[2][] grid;
    Point[] points;
    string[] texts;
}
// ----
// .resources.types | keys: ["t_array$_t_array$_t_uint256_$2_storage_$dyn_storage","t_array$_t_string_storage_$dyn_storage","t_array$_t_struct$_Point_$6_storage_$dyn_storage","t_array$_t_uint256_$2_storage","t_array$_t_uint256_$dyn_storage","t_array$_t_uint8_$dyn_storage","t_string_storage","t_struct$_Point_$6_storage","t_uint256","t_uint8"]
// .resources.pointers | keys: ["storage_25_12","storage_25_17","storage_25_21","storage_25_24","storage_25_9"]
// .resources.pointers.storage_25_12: {
//     "expect": [],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "packed-length",
//                 "slot": "0x01"
//             },
//             {
//                 "define": {
//                     "packed-data": {
//                         "$keccak256": [
//                             {
//                                 "$wordsized": "0x01"
//                             }
//                         ]
//                     }
//                 },
//                 "in": {
//                     "list": {
//                         "count": {
//                             "$read": "packed-length"
//                         },
//                         "each": "packed-index",
//                         "is": {
//                             "length": "0x01",
//                             "location": "storage",
//                             "name": "packed-item",
//                             "offset": {
//                                 "$difference": [
//                                     "$wordsize",
//                                     {
//                                         "$product": [
//                                             {
//                                                 "$sum": [
//                                                     {
//                                                         "$remainder": [
//                                                             "packed-index",
//                                                             "0x20"
//                                                         ]
//                                                     },
//                                                     "0x01"
//                                                 ]
//                                             },
//                                             "0x01"
//                                         ]
//                                     }
//                                 ]
//                             },
//                             "slot": {
//                                 "$sum": [
//                                     "packed-data",
//                                     {
//                                         "$quotient": [
//                                             "packed-index",
//                                             "0x20"
//                                         ]
//                                     }
//                                 ]
//                             }
//                         }
//                     }
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.storage_25_17: {
//     "expect": [],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "grid-length",
//                 "slot": "0x02"
//             },
//             {
//                 "define": {
//                     "grid-data": {
//                         "$keccak256": [
//                             {
//                                 "$wordsized": "0x02"
//                             }
//                         ]
//                     }
//                 },
//                 "in": {
//                     "list": {
//                         "count": {
//                             "$read": "grid-length"
//                         },
//                         "each": "grid-index",
//                         "is": {
//                             "list": {
//                                 "count": "0x02",
//                                 "each": "grid-item-index",
//                                 "is": {
//                                     "location": "storage",
//                                     "name": "grid-item-item",
//                                     "slot": {
//                                         "$sum": [
//                                             {
//                                                 "$sum": [
//                                                     "grid-data",
//                                                     {
//                                                         "$product": [
//                                                             "grid-index",
//                                                             "0x02"
//                                                         ]
//                                                     }
//                                                 ]
//                                             },
//                                             "grid-item-index"
//                                         ]
//                                     }
//                                 }
//                             }
//                         }
//                     }
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.storage_25_21: {
//     "expect": [],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "points-length",
//                 "slot": "0x03"
//             },
//             {
//                 "define": {
//                     "points-data": {
//                         "$keccak256": [
//                             {
//                                 "$wordsized": "0x03"
//                             }
//                         ]
//                     }
//                 },
//                 "in": {
//                     "list": {
//                         "count": {
//                             "$read": "points-length"
//                         },
//                         "each": "points-index",
//                         "is": {
//                             "group": [
//                                 {
//                                     "length": "0x01",
//                                     "location": "storage",
//                                     "name": "points-item-x",
//                                     "offset": "0x1f",
//                                     "slot": {
//                                         "$sum": [
//                                             "points-data",
//                                             "points-index"
//                                         ]
//                                     }
//                                 },
//                                 {
//                                     "length": "0x01",
//                                     "location": "storage",
//                                     "name": "points-item-y",
//                                     "offset": "0x1e",
//                                     "slot": {
//                                         "$sum": [
//                                             "points-data",
//                                             "points-index"
//                                         ]
//                                     }
//                                 }
//                             ]
//                         }
//                     }
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.storage_25_24: {
//     "expect": [],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "texts-length",
//                 "slot": "0x04"
//             },
//             {
//                 "define": {
//                     "texts-data": {
//                         "$keccak256": [
//                             {
//                                 "$wordsized": "0x04"
//                             }
//                         ]
//                     }
//                 },
//                 "in": {
//                     "list": {
//                         "count": {
//                             "$read": "texts-length"
//                         },
//                         "each": "texts-index",
//                         "is": {
//                             "group": [
//                                 {
//                                     "length": "0x01",
//                                     "location": "storage",
//                                     "name": "texts-item-length-flag",
//                                     "offset": {
//                                         "$difference": [
//                                             "$wordsize",
//                                             "0x01"
//                                         ]
//                                     },
//                                     "slot": {
//                                         "$sum": [
//                                             "texts-data",
//                                             "texts-index"
//                                         ]
//                                     }
//                                 },
//                                 {
//                                     "else": {
//                                         "group": [
//                                             {
//                                                 "location": "storage",
//                                                 "name": "texts-item-long-length",
//                                                 "slot": {
//                                                     "$sum": [
//                                                         "texts-data",
//                                                         "texts-index"
//                                                     ]
//                                                 }
//                                             },
//                                             {
//                                                 "define": {
//                                                     "texts-item-length": {
//                                                         "$quotient": [
//                                                             {
//                                                                 "$difference": [
//                                                                     {
//                                                                         "$read": "texts-item-long-length"
//                                                                     },
//                                                                     "0x01"
//                                                                 ]
//                                                             },
//                                                             "0x02"
//                                                         ]
//                                                     }
//                                                 },
//                                                 "in": {
//                                                     "define": {
//                                                         "texts-item-data": {
//                                                             "$keccak256": [
//                                                                 {
//                                                                     "$wordsized": {
//                                                                         "$sum": [
//                                                                             "texts-data",
//                                                                             "texts-index"
//                                                                         ]
//                                                                     }
//                                                                 }
//                                                             ]
//                                                         }
//                                                     },
//                                                     "in": {
//                                                         "length": "texts-item-length",
//                                                         "location": "storage",
//                                                         "name": "texts-item",
//                                                         "slot": "texts-item-data"
//                                                     }
//                                                 }
//                                             }
//                                         ]
//                                     },
//                                     "if": {
//                                         "$remainder": [
//                                             {
//                                                 "$sum": [
//                                                     {
//                                                         "$read": "texts-item-length-flag"
//                                                     },
//                                                     "0x01"
//                                                 ]
//                                             },
//                                             "0x02"
//                                         ]
//                                     },
//                                     "then": {
//                                         "define": {
//                                             "texts-item-length": {
//                                                 "$quotient": [
//                                                     {
//                                                         "$read": "texts-item-length-flag"
//                                                     },
//                                                     "0x02"
//                                                 ]
//                                             }
//                                         },
//                                         "in": {
//                                             "length": "texts-item-length",
//                                             "location": "storage",
//                                             "name": "texts-item",
//                                             "slot": {
//                                                 "$sum": [
//                                                     "texts-data",
//                                                     "texts-index"
//                                                 ]
//                                             }
//                                         }
//                                     }
//                                 }
//                             ]
//                         }
//                     }
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.storage_25_9: {
//     "expect": [],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "words-length",
//                 "slot": "0x00"
//             },
//             {
//                 "define": {
//                     "words-data": {
//                         "$keccak256": [
//                             {
//                                 "$wordsized": "0x00"
//                             }
//                         ]
//                     }
//                 },
//                 "in": {
//                     "list": {
//                         "count": {
//                             "$read": "words-length"
//                         },
//                         "each": "words-index",
//                         "is": {
//                             "location": "storage",
//                             "name": "words-item",
//                             "slot": {
//                                 "$sum": [
//                                     "words-data",
//                                     "words-index"
//                                 ]
//                             }
//                         }
//                     }
//                 }
//             }
//         ]
//     }
// }
