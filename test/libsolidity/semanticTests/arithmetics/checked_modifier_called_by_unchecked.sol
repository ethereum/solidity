contract C {
    modifier add(uint16 a, uint16 b) {
        unchecked { a + b; }
        _;
    }

    function f(uint16 a, uint16 b, uint16 c) public add(a, b) returns (uint16) {
        return b + c;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(uint16,uint16,uint16): 0xe000, 0xe500, 2 -> 58626
// f(uint16,uint16,uint16): 0x1000, 0xe500, 0xe000 -> FAILURE, hex"4e487b71", 0x11
