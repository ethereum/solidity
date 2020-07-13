contract C {
    struct S {
        uint8[] m;
    }
    S s;
    function f() public returns (bool correct) {
        s.m.push();
        assembly {
            mstore(0, s.slot)
            sstore(keccak256(0, 0x20), 257)
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
// compileViaYul: also
// ----
// f() -> true
