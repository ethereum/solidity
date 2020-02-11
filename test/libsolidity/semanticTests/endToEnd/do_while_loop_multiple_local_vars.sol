contract test {
    function f(uint x) public pure returns(uint r) {
        uint i = 0;
        do {
            uint z = x * 2;
            if (z < 4) break;
            else {
                uint k = z + 1;
                if (k < 8) {
                    x++;
                    continue;
                }
            }
            if (z > 12) return 0;
            x++;
            i++;
        } while (true);
        return 42;
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256): 0 -> 42
// f(uint256): 1 -> 42
// f(uint256): 2 -> 0
// f(uint256): 3 -> 0
// f(uint256): 4 -> 0
// f(uint256): 5 -> 0
// f(uint256): 6 -> 0
// f(uint256): 7 -> 0
// f(uint256): 8 -> 0
// f(uint256): 9 -> 0
// f(uint256): 10 -> 0
// f(uint256): 11 -> 0
