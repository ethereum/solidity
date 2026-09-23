struct S { uint256 $member; }

contract C {
    uint256 $value;
    S $struct;
    uint256[] $items;
}
// ----
// .resources.types | keys: ["t_array$_t_uint256_$dyn_storage","t_struct$_S_$4_storage","t_uint256"]
// .resources.pointers | keys: ["t_array$_t_uint256_$dyn_storage","t_struct$_S_$4_storage"]
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
// .resources.pointers.t_struct$_S_$4_storage: {
//     "expect": [
//         "slot"
//     ],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "_$member",
//                 "slot": "slot"
//             }
//         ]
//     }
// }
