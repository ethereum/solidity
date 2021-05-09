contract C {
    function f(uint256 a, uint256 b) public returns (uint256) {
        a >>= b;
        return a;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f(uint256,uint256): 0x4266, 0x0 -> 0x4266
// f(uint256,uint256): 0x4266, 0x8 -> 0x42
// f(uint256,uint256): 0x4266, 0x10 -> 0
// f(uint256,uint256): 0x4266, 0x11 -> 0
