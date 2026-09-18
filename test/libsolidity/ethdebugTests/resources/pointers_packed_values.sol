contract C {
    uint8 a;
    uint16 b;
    bool c;
    address d;
    uint256 e;
    bytes32 f;
    uint128 g;
    uint128 h;
    uint64 i;
}
// ----
// .resources.types | keys: ["t_address","t_bool","t_bytes32","t_uint128","t_uint16","t_uint256","t_uint64","t_uint8"]
// .resources.pointers | keys: ["storage_20_11","storage_20_13","storage_20_15","storage_20_17","storage_20_19","storage_20_3","storage_20_5","storage_20_7","storage_20_9"]
// .resources.pointers.storage_20_11: {
//     "expect": [],
//     "for": {
//         "location": "storage",
//         "name": "e",
//         "slot": "0x01"
//     }
// }
// .resources.pointers.storage_20_13: {
//     "expect": [],
//     "for": {
//         "location": "storage",
//         "name": "f",
//         "slot": "0x02"
//     }
// }
// .resources.pointers.storage_20_15: {
//     "expect": [],
//     "for": {
//         "length": "0x10",
//         "location": "storage",
//         "name": "g",
//         "offset": "0x10",
//         "slot": "0x03"
//     }
// }
// .resources.pointers.storage_20_17: {
//     "expect": [],
//     "for": {
//         "length": "0x10",
//         "location": "storage",
//         "name": "h",
//         "slot": "0x03"
//     }
// }
// .resources.pointers.storage_20_19: {
//     "expect": [],
//     "for": {
//         "length": "0x08",
//         "location": "storage",
//         "name": "i",
//         "offset": "0x18",
//         "slot": "0x04"
//     }
// }
// .resources.pointers.storage_20_3: {
//     "expect": [],
//     "for": {
//         "length": "0x01",
//         "location": "storage",
//         "name": "a",
//         "offset": "0x1f",
//         "slot": "0x00"
//     }
// }
// .resources.pointers.storage_20_5: {
//     "expect": [],
//     "for": {
//         "length": "0x02",
//         "location": "storage",
//         "name": "b",
//         "offset": "0x1d",
//         "slot": "0x00"
//     }
// }
// .resources.pointers.storage_20_7: {
//     "expect": [],
//     "for": {
//         "length": "0x01",
//         "location": "storage",
//         "name": "c",
//         "offset": "0x1c",
//         "slot": "0x00"
//     }
// }
// .resources.pointers.storage_20_9: {
//     "expect": [],
//     "for": {
//         "length": "0x14",
//         "location": "storage",
//         "name": "d",
//         "offset": "0x08",
//         "slot": "0x00"
//     }
// }
