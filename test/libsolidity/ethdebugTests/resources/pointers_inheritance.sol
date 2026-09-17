contract Base {
    uint256 inherited;
}

contract C is Base {
    uint256 own;
    uint256 transient temporary;
    uint256 constant CONSTANT = 1;
    uint256 immutable IMMUTABLE = 2;
}
// ====
// EVMVersion: >=cancun
// ----
// .resources.types.t_uint256: {"kind": "uint", "bits": 256}
// .resources.pointers | length: 4
