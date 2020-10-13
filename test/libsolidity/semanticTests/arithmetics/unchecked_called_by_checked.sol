contract C {
    function add(uint16 a, uint16 b) public returns (uint16) {
        unchecked {
            return a + b;
        }
    }

    function f(uint16 a) public returns (uint16) {
        return add(a, 0x100) + 0x100;
    }
}
// ====
// compileViaYul: also
// ----
// f(uint16): 7 -> 0x0207
// f(uint16): 0xffff -> 511
// f(uint16): 0xfeff -> FAILURE, hex"4e487b71", 0x11
