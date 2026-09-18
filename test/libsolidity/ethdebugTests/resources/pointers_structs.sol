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
// .resources.pointers | keys: ["storage_34_27","storage_34_30","storage_34_33"]
// .resources.pointers.storage_34_27: {
//     "expect": [],
//     "for": {
//         "group": [
//             {
//                 "length": "0x01",
//                 "location": "storage",
//                 "name": "point-x",
//                 "offset": "0x1f",
//                 "slot": "0x00"
//             },
//             {
//                 "length": "0x01",
//                 "location": "storage",
//                 "name": "point-y",
//                 "offset": "0x1e",
//                 "slot": "0x00"
//             },
//             {
//                 "length": "0x04",
//                 "location": "storage",
//                 "name": "point-salt",
//                 "offset": "0x1a",
//                 "slot": "0x00"
//             }
//         ]
//     }
// }
// .resources.pointers.storage_34_30: {
//     "expect": [],
//     "for": {
//         "group": [
//             {
//                 "group": [
//                     {
//                         "length": "0x01",
//                         "location": "storage",
//                         "name": "line-from-x",
//                         "offset": "0x1f",
//                         "slot": "0x01"
//                     },
//                     {
//                         "length": "0x01",
//                         "location": "storage",
//                         "name": "line-from-y",
//                         "offset": "0x1e",
//                         "slot": "0x01"
//                     },
//                     {
//                         "length": "0x04",
//                         "location": "storage",
//                         "name": "line-from-salt",
//                         "offset": "0x1a",
//                         "slot": "0x01"
//                     }
//                 ]
//             },
//             {
//                 "group": [
//                     {
//                         "length": "0x01",
//                         "location": "storage",
//                         "name": "line-to-x",
//                         "offset": "0x1f",
//                         "slot": "0x02"
//                     },
//                     {
//                         "length": "0x01",
//                         "location": "storage",
//                         "name": "line-to-y",
//                         "offset": "0x1e",
//                         "slot": "0x02"
//                     },
//                     {
//                         "length": "0x04",
//                         "location": "storage",
//                         "name": "line-to-salt",
//                         "offset": "0x1a",
//                         "slot": "0x02"
//                     }
//                 ]
//             },
//             {
//                 "group": [
//                     {
//                         "length": "0x01",
//                         "location": "storage",
//                         "name": "line-label-length-flag",
//                         "offset": {
//                             "$difference": [
//                                 "$wordsize",
//                                 "0x01"
//                             ]
//                         },
//                         "slot": "0x03"
//                     },
//                     {
//                         "else": {
//                             "group": [
//                                 {
//                                     "location": "storage",
//                                     "name": "line-label-long-length",
//                                     "slot": "0x03"
//                                 },
//                                 {
//                                     "define": {
//                                         "line-label-length": {
//                                             "$quotient": [
//                                                 {
//                                                     "$difference": [
//                                                         {
//                                                             "$read": "line-label-long-length"
//                                                         },
//                                                         "0x01"
//                                                     ]
//                                                 },
//                                                 "0x02"
//                                             ]
//                                         }
//                                     },
//                                     "in": {
//                                         "define": {
//                                             "line-label-data": {
//                                                 "$keccak256": [
//                                                     {
//                                                         "$wordsized": "0x03"
//                                                     }
//                                                 ]
//                                             }
//                                         },
//                                         "in": {
//                                             "length": "line-label-length",
//                                             "location": "storage",
//                                             "name": "line-label",
//                                             "slot": "line-label-data"
//                                         }
//                                     }
//                                 }
//                             ]
//                         },
//                         "if": {
//                             "$remainder": [
//                                 {
//                                     "$sum": [
//                                         {
//                                             "$read": "line-label-length-flag"
//                                         },
//                                         "0x01"
//                                     ]
//                                 },
//                                 "0x02"
//                             ]
//                         },
//                         "then": {
//                             "define": {
//                                 "line-label-length": {
//                                     "$quotient": [
//                                         {
//                                             "$read": "line-label-length-flag"
//                                         },
//                                         "0x02"
//                                     ]
//                                 }
//                             },
//                             "in": {
//                                 "length": "line-label-length",
//                                 "location": "storage",
//                                 "name": "line-label",
//                                 "slot": "0x03"
//                             }
//                         }
//                     }
//                 ]
//             }
//         ]
//     }
// }
// .resources.pointers.storage_34_33: {
//     "expect": [
//         "key"
//     ],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "registry-count",
//                 "slot": "0x04"
//             },
//             {
//                 "location": "storage",
//                 "name": "registry-index",
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
//             }
//         ]
//     }
// }
