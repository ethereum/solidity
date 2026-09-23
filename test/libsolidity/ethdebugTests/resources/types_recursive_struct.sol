struct Node {
    uint256 value;
    Node[] children;
    mapping(uint256 => Node) byKey;
}

contract C {
    Node root;
}
// ----
// .resources.types | keys: ["t_array$_t_struct$_Node_$13_storage_$dyn_storage","t_mapping$_t_uint256_$_t_struct$_Node_$13_storage_$","t_struct$_Node_$13_storage","t_uint256"]
// .resources.types.t_array$_t_struct$_Node_$13_storage_$dyn_storage: {
//     "contains": {
//         "type": {
//             "id": "t_struct$_Node_$13_storage"
//         }
//     },
//     "kind": "array"
// }
// .resources.types.t_mapping$_t_uint256_$_t_struct$_Node_$13_storage_$: {
//     "contains": {
//         "key": {
//             "type": {
//                 "id": "t_uint256"
//             }
//         },
//         "value": {
//             "type": {
//                 "id": "t_struct$_Node_$13_storage"
//             }
//         }
//     },
//     "kind": "mapping"
// }
// .resources.types.t_struct$_Node_$13_storage: {
//     "contains": [
//         {
//             "name": "value",
//             "type": {
//                 "id": "t_uint256"
//             }
//         },
//         {
//             "name": "children",
//             "type": {
//                 "id": "t_array$_t_struct$_Node_$13_storage_$dyn_storage"
//             }
//         },
//         {
//             "name": "byKey",
//             "type": {
//                 "id": "t_mapping$_t_uint256_$_t_struct$_Node_$13_storage_$"
//             }
//         }
//     ],
//     "definition": {
//         "location": {
//             "range": {
//                 "length": 91,
//                 "offset": 59
//             },
//             "source": {
//                 "id": 0
//             }
//         },
//         "name": "Node"
//     },
//     "kind": "struct"
// }
// .resources.types.t_uint256: {"bits":256,"kind":"uint"}
// .resources.pointers | keys: ["t_array$_t_struct$_Node_$13_storage_$dyn_storage","t_mapping$_t_uint256_$_t_struct$_Node_$13_storage_$","t_struct$_Node_$13_storage"]
