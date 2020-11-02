contract C {
    function f(uint32 a, uint32 b) public returns (uint256) {
        return a >> b;
    }
}

// ====
// compileViaYul: also
// ----
// f(uint32,uint32): 0x4266, 0x0 -> 0x4266
// f(uint32,uint32): 0x4266, 0x8 -> 0x42
// f(uint32,uint32): 0x4266, 0x10 -> 0
// f(uint32,uint32): 0x4266, 0x11 -> 0
