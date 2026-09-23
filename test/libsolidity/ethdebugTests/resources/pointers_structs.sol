struct Point { uint8 x; uint8 y; bytes4 salt; }
struct Line { Point from; Point to; string label; }
struct Registry { uint256 count; mapping(address => uint256) index; }

contract C {
    Point point;
    Line line;
    Registry registry;
}
// ----
// .resources.types | keys: ["t_address","t_bytes4","t_mapping$_t_address_$_t_uint256_$","t_string_storage","t_struct$_Line_$17_storage","t_struct$_Point_$8_storage","t_struct$_Registry_$24_storage","t_uint256","t_uint8"]
// .resources.pointers | keys: ["t_mapping$_t_address_$_t_uint256_$","t_string_storage","t_struct$_Line_$17_storage","t_struct$_Point_$8_storage","t_struct$_Registry_$24_storage"]
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
// .resources.pointers.t_struct$_Line_$17_storage: {
//     "expect": [
//         "slot"
//     ],
//     "for": {
//         "group": [
//             {
//                 "template": "t_struct$_Point_$8_storage",
//                 "yields": {
//                     "salt": "from-salt",
//                     "x": "from-x",
//                     "y": "from-y"
//                 }
//             },
//             {
//                 "define": {
//                     "slot": {
//                         "$sum": [
//                             "slot",
//                             "0x01"
//                         ]
//                     }
//                 },
//                 "in": {
//                     "template": "t_struct$_Point_$8_storage",
//                     "yields": {
//                         "salt": "to-salt",
//                         "x": "to-x",
//                         "y": "to-y"
//                     }
//                 }
//             },
//             {
//                 "define": {
//                     "slot": {
//                         "$sum": [
//                             "slot",
//                             "0x02"
//                         ]
//                     }
//                 },
//                 "in": {
//                     "template": "t_string_storage",
//                     "yields": {
//                         "data": "label-data",
//                         "length-flag": "label-length-flag",
//                         "long-length": "label-long-length"
//                     }
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.t_struct$_Point_$8_storage: {
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
//             },
//             {
//                 "length": "0x04",
//                 "location": "storage",
//                 "name": "salt",
//                 "offset": "0x1a",
//                 "slot": "slot"
//             }
//         ]
//     }
// }
// .resources.pointers.t_struct$_Registry_$24_storage: {
//     "expect": [
//         "slot"
//     ],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "count",
//                 "slot": "slot"
//             },
//             {
//                 "location": "storage",
//                 "name": "index",
//                 "slot": {
//                     "$sum": [
//                         "slot",
//                         "0x01"
//                     ]
//                 }
//             }
//         ]
//     }
// }
