contract C {
    uint8 small;
    bool flag;
    int256 signed;
    address owner;
    address payable sink;
    bytes32 hash;
    bytes blob;
    string text;
    function f(uint16 a, bytes4 b) public pure returns (bool) { return a > 0 && b != 0; }
}
// ----
// .resources.types | keys: ["t_address","t_address_payable","t_bool","t_bytes32","t_bytes4","t_bytes_storage","t_int256","t_string_storage","t_uint16","t_uint8"]
// .resources.types.t_address: {"kind":"address","payable":false}
// .resources.types.t_address_payable: {"kind":"address","payable":true}
// .resources.types.t_bool: {"kind":"bool"}
// .resources.types.t_bytes32: {"kind":"bytes","size":32}
// .resources.types.t_bytes4: {"kind":"bytes","size":4}
// .resources.types.t_bytes_storage: {"kind":"bytes"}
// .resources.types.t_int256: {"bits":256,"kind":"int"}
// .resources.types.t_string_storage: {"kind":"string"}
// .resources.types.t_uint16: {"bits":16,"kind":"uint"}
// .resources.types.t_uint8: {"bits":8,"kind":"uint"}
// .resources.pointers | keys: ["t_bytes_storage","t_string_storage"]
