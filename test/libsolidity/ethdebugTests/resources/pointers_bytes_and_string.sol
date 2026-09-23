contract C {
    bytes blob;
    string text;
}
// ----
// .resources.types | keys: ["t_bytes_storage","t_string_storage"]
// .resources.pointers | keys: ["t_bytes_storage","t_string_storage"]
// .resources.pointers.t_bytes_storage: {
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
