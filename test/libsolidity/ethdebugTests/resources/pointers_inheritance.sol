contract Base {
    uint256 inherited;
    uint8 packedBase;
}

contract C is Base {
    uint8 packedOwn;
    uint256 own;
    uint256 constant CONSTANT = 1;
    uint256 immutable IMMUTABLE = 2;
}

contract D is Base {
    string own;
}
// ----
// .resources.types | keys: ["t_string_storage","t_uint256","t_uint8"]
// .resources.pointers | keys: ["storage_19_10","storage_19_12","storage_19_3","storage_19_5","storage_24_23","storage_24_3","storage_24_5","storage_6_3","storage_6_5"]
// .resources.pointers.storage_19_10: {
//     "expect": [],
//     "for": {
//         "length": "0x01",
//         "location": "storage",
//         "name": "packedOwn",
//         "offset": "0x1e",
//         "slot": "0x01"
//     }
// }
// .resources.pointers.storage_19_12: {
//     "expect": [],
//     "for": {
//         "location": "storage",
//         "name": "own",
//         "slot": "0x02"
//     }
// }
// .resources.pointers.storage_19_3: {
//     "expect": [],
//     "for": {
//         "location": "storage",
//         "name": "inherited",
//         "slot": "0x00"
//     }
// }
// .resources.pointers.storage_19_5: {
//     "expect": [],
//     "for": {
//         "length": "0x01",
//         "location": "storage",
//         "name": "packedBase",
//         "offset": "0x1f",
//         "slot": "0x01"
//     }
// }
// .resources.pointers.storage_24_23: {
//     "expect": [],
//     "for": {
//         "group": [
//             {
//                 "length": "0x01",
//                 "location": "storage",
//                 "name": "own-length-flag",
//                 "offset": {
//                     "$difference": [
//                         "$wordsize",
//                         "0x01"
//                     ]
//                 },
//                 "slot": "0x02"
//             },
//             {
//                 "else": {
//                     "group": [
//                         {
//                             "location": "storage",
//                             "name": "own-long-length",
//                             "slot": "0x02"
//                         },
//                         {
//                             "define": {
//                                 "own-length": {
//                                     "$quotient": [
//                                         {
//                                             "$difference": [
//                                                 {
//                                                     "$read": "own-long-length"
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
//                                     "own-data": {
//                                         "$keccak256": [
//                                             {
//                                                 "$wordsized": "0x02"
//                                             }
//                                         ]
//                                     }
//                                 },
//                                 "in": {
//                                     "length": "own-length",
//                                     "location": "storage",
//                                     "name": "own",
//                                     "slot": "own-data"
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
//                                     "$read": "own-length-flag"
//                                 },
//                                 "0x01"
//                             ]
//                         },
//                         "0x02"
//                     ]
//                 },
//                 "then": {
//                     "define": {
//                         "own-length": {
//                             "$quotient": [
//                                 {
//                                     "$read": "own-length-flag"
//                                 },
//                                 "0x02"
//                             ]
//                         }
//                     },
//                     "in": {
//                         "length": "own-length",
//                         "location": "storage",
//                         "name": "own",
//                         "slot": "0x02"
//                     }
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.storage_24_3: {
//     "expect": [],
//     "for": {
//         "location": "storage",
//         "name": "inherited",
//         "slot": "0x00"
//     }
// }
// .resources.pointers.storage_24_5: {
//     "expect": [],
//     "for": {
//         "length": "0x01",
//         "location": "storage",
//         "name": "packedBase",
//         "offset": "0x1f",
//         "slot": "0x01"
//     }
// }
// .resources.pointers.storage_6_3: {
//     "expect": [],
//     "for": {
//         "location": "storage",
//         "name": "inherited",
//         "slot": "0x00"
//     }
// }
// .resources.pointers.storage_6_5: {
//     "expect": [],
//     "for": {
//         "length": "0x01",
//         "location": "storage",
//         "name": "packedBase",
//         "offset": "0x1f",
//         "slot": "0x01"
//     }
// }
