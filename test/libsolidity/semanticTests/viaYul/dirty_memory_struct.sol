contract C {
    struct S {
        uint8[] m;
    }
    function f() public pure returns (bool correct) {
        S memory s;
        s.m = new uint8[](1);
        assembly {
            mstore(add(s, 64), 257)
        }
        uint8 x = s.m[0];
        uint r;
        assembly {
            r := x
        }
        correct = r == 0x01;
    }
}
// ====
// compileViaYul: true
// ----
// f() -> true
