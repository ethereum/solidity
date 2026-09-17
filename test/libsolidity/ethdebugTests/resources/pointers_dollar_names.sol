struct S { uint256 $member; }

contract C {
    uint256 $value;
    S $struct;
    uint256[] $items;
}
// ----
// .resources.types | keys: ["t_array$_t_uint256_$dyn_storage","t_struct$_S_$4_storage","t_uint256"]
// .resources.pointers | keys: ["storage_13_12","storage_13_6","storage_13_9"]
// .resources.pointers.storage_13_12: {
//     "expect": [],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "_$items-length",
//                 "slot": "0x02"
//             },
//             {
//                 "define": {
//                     "_$items-data": {
//                         "$keccak256": [
//                             {
//                                 "$wordsized": "0x02"
//                             }
//                         ]
//                     }
//                 },
//                 "in": {
//                     "list": {
//                         "count": {
//                             "$read": "_$items-length"
//                         },
//                         "each": "_$items-index",
//                         "is": {
//                             "location": "storage",
//                             "name": "_$items-item",
//                             "slot": {
//                                 "$sum": [
//                                     "_$items-data",
//                                     "_$items-index"
//                                 ]
//                             }
//                         }
//                     }
//                 }
//             }
//         ]
//     }
// }
// .resources.pointers.storage_13_6: {
//     "expect": [],
//     "for": {
//         "location": "storage",
//         "name": "_$value",
//         "slot": "0x00"
//     }
// }
// .resources.pointers.storage_13_9: {
//     "expect": [],
//     "for": {
//         "group": [
//             {
//                 "location": "storage",
//                 "name": "_$struct-$member",
//                 "slot": "0x01"
//             }
//         ]
//     }
// }
