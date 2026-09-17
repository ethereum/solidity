contract C {
    uint256 transient word;
    uint8 transient a;
    bool transient b;
    address transient c;
    uint256 stored;
}
// ====
// EVMVersion: >=cancun
// ----
// .resources.types | keys: ["t_address","t_bool","t_uint256","t_uint8"]
// .resources.pointers | keys: ["storage_12_11","transient_12_3","transient_12_5","transient_12_7","transient_12_9"]
// .resources.pointers.storage_12_11: {
//     "expect": [],
//     "for": {
//         "location": "storage",
//         "name": "stored",
//         "slot": "0x00"
//     }
// }
// .resources.pointers.transient_12_3: {
//     "expect": [],
//     "for": {
//         "location": "transient",
//         "name": "word",
//         "slot": "0x00"
//     }
// }
// .resources.pointers.transient_12_5: {
//     "expect": [],
//     "for": {
//         "length": "0x01",
//         "location": "transient",
//         "name": "a",
//         "slot": "0x01"
//     }
// }
// .resources.pointers.transient_12_7: {
//     "expect": [],
//     "for": {
//         "length": "0x01",
//         "location": "transient",
//         "name": "b",
//         "offset": "0x01",
//         "slot": "0x01"
//     }
// }
// .resources.pointers.transient_12_9: {
//     "expect": [],
//     "for": {
//         "length": "0x14",
//         "location": "transient",
//         "name": "c",
//         "offset": "0x02",
//         "slot": "0x01"
//     }
// }
