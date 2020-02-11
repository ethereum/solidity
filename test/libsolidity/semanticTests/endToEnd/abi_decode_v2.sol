pragma experimental ABIEncoderV2;
contract C {
    struct S {
        uint a;
        uint[] b;
    }

    function f() public pure returns(S memory) {
        S memory s;
        s.a = 8;
        s.b = new uint[](3);
        s.b[0] = 9;
        s.b[1] = 10;
        s.b[2] = 11;
        return abi.decode(abi.encode(s), (S));
    }
}

// ----
// f() -> 32, 8, 64, 3, 9, 10, 11
