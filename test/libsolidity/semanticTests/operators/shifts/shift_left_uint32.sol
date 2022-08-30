contract C {
    function f(uint32 a, uint32 b) public returns (uint256) {
        return a << b;
    }
}

// ====
// compileToEwasm: also
// ----
// f(uint32,uint32): 0x4266, 0x0 -> 0x4266
// f(uint32,uint32): 0x4266, 0x8 -> 0x426600
// f(uint32,uint32): 0x4266, 0x10 -> 0x42660000
// f(uint32,uint32): 0x4266, 0x11 -> 0x84cc0000
// f(uint32,uint32): 0x4266, 0x20 -> 0
