contract test {
    function f(uint x) public pure returns(uint r) {
        for (uint i = 0; i < 12; i++) {
            uint z = x + 1;
            if (z < 4) break;
            else {
                uint k = z * 2;
                if (i + k < 10) {
                    x++;
                    continue;
                }
            }
            if (z > 8) return 0;
            x++;
        }
        return 42;
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256): 0 -> 42
// f(uint256): 1 -> 42
// f(uint256): 2 -> 42
// f(uint256): 3 -> 0
// f(uint256): 4 -> 0
// f(uint256): 5 -> 0
// f(uint256): 6 -> 0
// f(uint256): 7 -> 0
// f(uint256): 8 -> 0
// f(uint256): 9 -> 0
// f(uint256): 10 -> 0
// f(uint256): 11 -> 0
