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
// .resources.pointers | keys: ["t_array$_t_array$_t_uint256_$2_storage_$dyn_storage","t_array$_t_string_storage_$dyn_storage","t_array$_t_struct$_Point_$6_storage_$dyn_storage","t_array$_t_uint256_$2_storage","t_array$_t_uint256_$dyn_storage","t_array$_t_uint8_$dyn_storage","t_string_storage","t_struct$_Point_$6_storage"]
// .resources.pointers.t_array$_t_array$_t_uint256_$2_storage_$dyn_storage: {
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
//                             "define": {
//                                 "slot": {
//                                     "$sum": [
//                                         "data",
//                                         {
//                                             "$product": [
//                                                 "index",
//                                                 "0x02"
//                                             ]
//                                         }
//                                     ]
//                                 }
//                             },
//                             "in": {
//                                 "template": "t_array$_t_uint256_$2_storage",
//                                 "yields": {
//                                     "item": "item-item"
//                                 }
//                             }
//                         }
//                     }
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.t_array$_t_string_storage_$dyn_storage: {
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
//                             "define": {
//                                 "slot": {
//                                     "$sum": [
//                                         "data",
//                                         "index"
//                                     ]
//                                 }
//                             },
//                             "in": {
//                                 "template": "t_string_storage",
//                                 "yields": {
//                                     "data": "item-data",
//                                     "length-flag": "item-length-flag",
//                                     "long-length": "item-long-length"
//                                 }
//                             }
//                         }
//                     }
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.t_array$_t_struct$_Point_$6_storage_$dyn_storage: {
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
//                             "define": {
//                                 "slot": {
//                                     "$sum": [
//                                         "data",
//                                         "index"
//                                     ]
//                                 }
//                             },
//                             "in": {
//                                 "template": "t_struct$_Point_$6_storage",
//                                 "yields": {
//                                     "x": "item-x",
//                                     "y": "item-y"
//                                 }
//                             }
//                         }
//                     }
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.t_array$_t_uint256_$2_storage: {
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
// .resources.pointers.t_array$_t_uint8_$dyn_storage: {
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
//                             "length": "0x01",
//                             "location": "storage",
//                             "name": "item",
//                             "offset": {
//                                 "$difference": [
//                                     "$wordsize",
//                                     {
//                                         "$sum": [
//                                             {
//                                                 "$remainder": [
//                                                     "index",
//                                                     "0x20"
//                                                 ]
//                                             },
//                                             "0x01"
//                                         ]
//                                     }
//                                 ]
//                             },
//                             "slot": {
//                                 "$sum": [
//                                     "data",
//                                     {
//                                         "$quotient": [
//                                             "index",
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
// .resources.pointers.t_string_storage: {
//     "expect": [
//         "slot"
//     ],
//     "for": {
//         "group": [
//             {
//                 "length": "0x01",
//                 "location": "storage",
//                 "name": "length-flag",
//                 "offset": {
//                     "$difference": [
//                         "$wordsize",
//                         "0x01"
//                     ]
//                 },
//                 "slot": "slot"
//             },
//             {
//                 "else": {
//                     "group": [
//                         {
//                             "location": "storage",
//                             "name": "long-length",
//                             "slot": "slot"
//                         },
//                         {
//                             "define": {
//                                 "length": {
//                                     "$quotient": [
//                                         {
//                                             "$difference": [
//                                                 {
//                                                     "$read": "long-length"
//                                                 },
//                                                 "0x01"
//                                             ]
//                                         },
//                                         "0x02"
//                                     ]
//                                 }
//                             },
//                             "in": {
//                                 "define": {
//                                     "start": {
//                                         "$keccak256": [
//                                             {
//                                                 "$wordsized": "slot"
//                                             }
//                                         ]
//                                     }
//                                 },
//                                 "in": {
//                                     "length": "length",
//                                     "location": "storage",
//                                     "name": "data",
//                                     "slot": "start"
//                                 }
//                             }
//                         }
//                     ]
//                 },
//                 "if": {
//                     "$remainder": [
//                         {
//                             "$sum": [
//                                 {
//                                     "$read": "length-flag"
//                                 },
//                                 "0x01"
//                             ]
//                         },
//                         "0x02"
//                     ]
//                 },
//                 "then": {
//                     "define": {
//                         "length": {
//                             "$quotient": [
//                                 {
//                                     "$read": "length-flag"
//                                 },
//                                 "0x02"
//                             ]
//                         }
//                     },
//                     "in": {
//                         "length": "length",
//                         "location": "storage",
//                         "name": "data",
//                         "slot": "slot"
//                     }
//                 }
//             }
//         ]
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
