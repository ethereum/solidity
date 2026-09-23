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
// .resources.pointers | keys: ["t_string_storage"]
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
