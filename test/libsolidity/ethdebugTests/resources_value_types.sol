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
// .resources.types | keys: <IGNORE>
// .resources.types.t_uint8: {"kind": "uint", "bits": 8}
// .resources.types.t_uint16: {"kind": "uint", "bits": 16}
// .resources.types.t_int256: {"kind": "int", "bits": 256}
// .resources.types.t_bool: {"kind": "bool"}
// .resources.types.t_address: {"kind": "address", "payable": false}
// .resources.types.t_address_payable: {"kind": "address", "payable": true}
// .resources.types.t_bytes32: {"kind": "bytes", "size": 32}
// .resources.types.t_bytes4: {"kind": "bytes", "size": 4}
// .resources.types.t_bytes_storage: {"kind": "bytes"}
// .resources.types.t_string_storage: {"kind": "string"}
// .resources.pointers | length: 8
