contract C {
    bytes blob;
    string text;
}
// ----
// .resources.types | keys: ["t_bytes_storage","t_string_storage"]
// .resources.pointers | keys: ["storage_6_3","storage_6_5"]
// .resources.pointers.storage_6_3: {
//     "expect": [],
//     "for": {
//         "group": [
//             {
//                 "length": "0x01",
//                 "location": "storage",
//                 "name": "blob-length-flag",
//                 "offset": {
//                     "$difference": [
//                         "$wordsize",
//                         "0x01"
//                     ]
//                 },
//                 "slot": "0x00"
//             },
//             {
//                 "else": {
//                     "group": [
//                         {
//                             "location": "storage",
//                             "name": "blob-long-length",
//                             "slot": "0x00"
//                         },
//                         {
//                             "define": {
//                                 "blob-length": {
//                                     "$quotient": [
//                                         {
//                                             "$difference": [
//                                                 {
//                                                     "$read": "blob-long-length"
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
//                                     "blob-data": {
//                                         "$keccak256": [
//                                             {
//                                                 "$wordsized": "0x00"
//                                             }
//                                         ]
//                                     }
//                                 },
//                                 "in": {
//                                     "length": "blob-length",
//                                     "location": "storage",
//                                     "name": "blob",
//                                     "slot": "blob-data"
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
//                                     "$read": "blob-length-flag"
//                                 },
//                                 "0x01"
//                             ]
//                         },
//                         "0x02"
//                     ]
//                 },
//                 "then": {
//                     "define": {
//                         "blob-length": {
//                             "$quotient": [
//                                 {
//                                     "$read": "blob-length-flag"
//                                 },
//                                 "0x02"
//                             ]
//                         }
//                     },
//                     "in": {
//                         "length": "blob-length",
//                         "location": "storage",
//                         "name": "blob",
//                         "slot": "0x00"
//                     }
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.storage_6_5: {
//     "expect": [],
//     "for": {
//         "group": [
//             {
//                 "length": "0x01",
//                 "location": "storage",
//                 "name": "text-length-flag",
//                 "offset": {
//                     "$difference": [
//                         "$wordsize",
//                         "0x01"
//                     ]
//                 },
//                 "slot": "0x01"
//             },
//             {
//                 "else": {
//                     "group": [
//                         {
//                             "location": "storage",
//                             "name": "text-long-length",
//                             "slot": "0x01"
//                         },
//                         {
//                             "define": {
//                                 "text-length": {
//                                     "$quotient": [
//                                         {
//                                             "$difference": [
//                                                 {
//                                                     "$read": "text-long-length"
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
//                                     "text-data": {
//                                         "$keccak256": [
//                                             {
//                                                 "$wordsized": "0x01"
//                                             }
//                                         ]
//                                     }
//                                 },
//                                 "in": {
//                                     "length": "text-length",
//                                     "location": "storage",
//                                     "name": "text",
//                                     "slot": "text-data"
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
//                                     "$read": "text-length-flag"
//                                 },
//                                 "0x01"
//                             ]
//                         },
//                         "0x02"
//                     ]
//                 },
//                 "then": {
//                     "define": {
//                         "text-length": {
//                             "$quotient": [
//                                 {
//                                     "$read": "text-length-flag"
//                                 },
//                                 "0x02"
//                             ]
//                         }
//                     },
//                     "in": {
//                         "length": "text-length",
//                         "location": "storage",
//                         "name": "text",
//                         "slot": "0x01"
//                     }
//                 }
//             }
//         ]
//     }
// }
