pragma abicoder               v2;


contract C {
    struct S {
        uint256 a;
        uint256[] b;
    }

    function f(bytes calldata data) external pure returns (S memory) {
        return abi.decode(data, (S));
    }
}

// ----
// f(bytes): 0x20, 0xe0, 0x20, 0x21, 0x40, 0x3, 0xa, 0xb, 0xc -> 0x20, 0x21, 0x40, 0x3, 0xa, 0xb, 0xc
