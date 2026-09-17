contract C {
    uint16[8] packed;
    uint256[] values;
    uint256[2][] grid;
    bytes blob;
    string text;
}
// ----
// .resources.types | keys: ["t_array$_t_array$_t_uint256_$2_storage_$dyn_storage","t_array$_t_uint16_$8_storage","t_array$_t_uint256_$2_storage","t_array$_t_uint256_$dyn_storage","t_bytes_storage","t_string_storage","t_uint16","t_uint256"]
// .resources.types.t_array$_t_array$_t_uint256_$2_storage_$dyn_storage: {
//     "contains": {
//         "type": {
//             "id": "t_array$_t_uint256_$2_storage"
//         }
//     },
//     "kind": "array"
// }
// .resources.types.t_array$_t_uint16_$8_storage: {
//     "contains": {
//         "type": {
//             "id": "t_uint16"
//         }
//     },
//     "count": "0x08",
//     "kind": "array"
// }
// .resources.types.t_array$_t_uint256_$2_storage: {
//     "contains": {
//         "type": {
//             "id": "t_uint256"
//         }
//     },
//     "count": "0x02",
//     "kind": "array"
// }
// .resources.types.t_array$_t_uint256_$dyn_storage: {
//     "contains": {
//         "type": {
//             "id": "t_uint256"
//         }
//     },
//     "kind": "array"
// }
// .resources.types.t_bytes_storage: {"kind":"bytes"}
// .resources.types.t_string_storage: {"kind":"string"}
// .resources.types.t_uint16: {"bits":16,"kind":"uint"}
// .resources.types.t_uint256: {"bits":256,"kind":"uint"}
// .resources.pointers | keys: ["storage_18_13","storage_18_15","storage_18_17","storage_18_5","storage_18_8"]
