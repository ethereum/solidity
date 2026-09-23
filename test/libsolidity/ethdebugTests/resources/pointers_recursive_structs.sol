struct Node {
    uint256 value;
    uint256 extra;
    Node[] children;
}

struct Leaf {
    uint256 value;
    mapping(uint256 => Leaf) byKey;
}

contract C {
    Node tree;
    Leaf leaf;
}
// ----
// .resources.types | keys: ["t_array$_t_struct$_Node_$10_storage_$dyn_storage","t_mapping$_t_uint256_$_t_struct$_Leaf_$18_storage_$","t_struct$_Leaf_$18_storage","t_struct$_Node_$10_storage","t_uint256"]
// .resources.pointers | keys: ["t_array$_t_struct$_Node_$10_storage_$dyn_storage","t_mapping$_t_uint256_$_t_struct$_Leaf_$18_storage_$","t_struct$_Leaf_$18_storage","t_struct$_Node_$10_storage"]
// .resources.pointers.t_array$_t_struct$_Node_$10_storage_$dyn_storage: {
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
//                                                 "0x03"
//                                             ]
//                                         }
//                                     ]
//                                 }
//                             },
//                             "in": {
//                                 "template": "t_struct$_Node_$10_storage"
//                             }
//                         }
//                     }
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.t_mapping$_t_uint256_$_t_struct$_Leaf_$18_storage_$: {
//     "expect": [
//         "slot",
//         "key"
//     ],
//     "for": {
//         "define": {
//             "slot": {
//                 "$keccak256": [
//                     {
//                         "$wordsized": "key"
//                     },
//                     {
//                         "$wordsized": "slot"
//                     }
//                 ]
//             }
//         },
//         "in": {
//             "template": "t_struct$_Leaf_$18_storage"
//         }
//     }
// }
// .resources.pointers.t_struct$_Leaf_$18_storage: {
//     "expect": [
//         "slot"
//     ],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "value",
//                 "slot": "slot"
//             },
//             {
//                 "location": "storage",
//                 "name": "byKey",
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
// .resources.pointers.t_struct$_Node_$10_storage: {
//     "expect": [
//         "slot"
//     ],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "value",
//                 "slot": "slot"
//             },
//             {
//                 "location": "storage",
//                 "name": "extra",
//                 "slot": {
//                     "$sum": [
//                         "slot",
//                         "0x01"
//                     ]
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
//                     "template": "t_array$_t_struct$_Node_$10_storage_$dyn_storage",
//                     "yields": {
//                         "length": "children-length"
//                     }
//                 }
//             }
//         ]
//     }
// }
