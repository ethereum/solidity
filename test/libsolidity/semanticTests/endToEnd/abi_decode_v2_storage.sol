pragma experimental ABIEncoderV2;
contract C {
    bytes data;
    struct S {
        uint a;
        uint[] b;
    }

    function f() public returns(S memory) {
        S memory s;
        s.a = 8;
        s.b = new uint[](3);
        s.b[0] = 9;
        s.b[1] = 10;
        s.b[2] = 11;
        data = abi.encode(s);
        return abi.decode(data, (S));
    }
}

// ----
// f() -> 0x20, 8, 0x40, 3, 9, 10, 11
// f():"" -> "32, 8, 64, 3, 9, 10, 11"
