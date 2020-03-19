pragma experimental ABIEncoderV2;


contract C {
    struct S {
        uint256 a;
        uint256[] b;
    }

    function f() public pure returns (S memory) {
        S memory s;
        s.a = 8;
        s.b = new uint256[](3);
        s.b[0] = 9;
        s.b[1] = 10;
        s.b[2] = 11;
        return abi.decode(abi.encode(s), (S));
    }
}

// ----
// f() -> 0x20, 0x8, 0x40, 0x3, 0x9, 0xa, 0xb
