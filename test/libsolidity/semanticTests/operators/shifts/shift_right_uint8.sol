contract C {
    function f(uint8 a, uint8 b) public returns (uint256) {
        return a >> b;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f(uint8,uint8): 0x66, 0x0 -> 0x66
// f(uint8,uint8): 0x66, 0x8 -> 0x0
