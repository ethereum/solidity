contract C {
    bytes b;
    function f() public returns (bool correct) {
        assembly {
            sstore(b.slot, 0x41)
            mstore(0, b.slot)
            sstore(keccak256(0, 0x20), "deadbeefdeadbeefdeadbeefdeadbeef")
        }
        byte s = b[31];
        uint r;
        assembly {
            r := s
        }
        correct = r == (0x66 << 248);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> true