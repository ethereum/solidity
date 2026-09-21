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
// .resources.pointers | keys: ["storage_25_21","storage_25_24"]
// .resources.pointers.storage_25_21: {
//     "expect": [],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "tree-value",
//                 "slot": "0x00"
//             },
//             {
//                 "location": "storage",
//                 "name": "tree-extra",
//                 "slot": "0x01"
//             },
//             {
//                 "group": [
//                     {
//                         "location": "storage",
//                         "name": "tree-children-length",
//                         "slot": "0x02"
//                     },
//                     {
//                         "define": {
//                             "tree-children-data": {
//                                 "$keccak256": [
//                                     {
//                                         "$wordsized": "0x02"
//                                     }
//                                 ]
//                             }
//                         },
//                         "in": {
//                             "list": {
//                                 "count": {
//                                     "$read": "tree-children-length"
//                                 },
//                                 "each": "tree-children-index",
//                                 "is": {
//                                     "length": "0x60",
//                                     "location": "storage",
//                                     "name": "tree-children-item",
//                                     "slot": {
//                                         "$sum": [
//                                             "tree-children-data",
//                                             {
//                                                 "$product": [
//                                                     "tree-children-index",
//                                                     "0x03"
//                                                 ]
//                                             }
//                                         ]
//                                     }
//                                 }
//                             }
//                         }
//                     }
//                 ]
//             }
//         ]
//     }
// }
// .resources.pointers.storage_25_24: {
//     "expect": [
//         "key"
//     ],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "leaf-value",
//                 "slot": "0x03"
//             },
//             {
//                 "length": "0x40",
//                 "location": "storage",
//                 "name": "leaf-byKey",
//                 "slot": {
//                     "$keccak256": [
//                         {
//                             "$wordsized": "key"
//                         },
//                         {
//                             "$wordsized": "0x04"
//                         }
//                     ]
//                 }
//             }
//         ]
//     }
// }
