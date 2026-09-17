contract C {
    uint16[8] packed;
    uint256[] values;
    uint256[2][] grid;
    bytes blob;
    string text;
}
// ----
// .resources.types.t_array$_t_uint16_$8_storage: {"kind": "array", "contains": {"type": {"id": "t_uint16"}}, "count": "0x08"}
// .resources.types.t_array$_t_uint256_$dyn_storage: {"kind": "array", "contains": {"type": {"id": "t_uint256"}}}
// .resources.types.t_array$_t_array$_t_uint256_$2_storage_$dyn_storage.contains.type.id: t_array$_t_uint256_$2_storage
// .resources.types.t_array$_t_uint256_$2_storage.count: 0x02
// .resources.pointers | length: 5
