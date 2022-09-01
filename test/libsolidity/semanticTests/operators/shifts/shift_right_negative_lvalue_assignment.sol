contract C {
    function f(int256 a, uint256 b) public returns (int256) {
        a >>= b;
        return a;
    }
}

// ====
// compileToEwasm: also
// ----
// f(int256,uint256): -4266, 0 -> -4266
// f(int256,uint256): -4266, 1 -> -2133
// f(int256,uint256): -4266, 4 -> -267
// f(int256,uint256): -4266, 8 -> -17
// f(int256,uint256): -4266, 16 -> -1
// f(int256,uint256): -4266, 17 -> -1
// f(int256,uint256): -4267, 0 -> -4267
// f(int256,uint256): -4267, 1 -> -2134
// f(int256,uint256): -4267, 4 -> -267
// f(int256,uint256): -4267, 8 -> -17
// f(int256,uint256): -4267, 16 -> -1
// f(int256,uint256): -4267, 17 -> -1
