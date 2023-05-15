contract C {
    function add(uint16 a, uint16 b) public returns (uint16) {
        return a + b;
    }

    function f(uint16 a, uint16 b, uint16 c) public returns (uint16) {
        unchecked { return add(a, b) + c; }
    }
}
// ----
// f(uint16,uint16,uint16): 0xe000, 0xe500, 2 -> FAILURE, hex"4e487b71", 0x11
// f(uint16,uint16,uint16): 0xe000, 0x1000, 0x1000 -> 0x00
