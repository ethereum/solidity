contract test {
    function f(uint x) public pure returns(uint r) {
        for (uint i = 0; i < 5; i++) {
            uint z = x + 1;
            if (z < 3) {
                break;
                uint p = z + 2;
            }
            for (uint j = 0; j < 5; j++) {
                uint k = z * 2;
                if (j + k < 8) {
                    x++;
                    continue;
                    uint t = z * 3;
                }
                x++;
                if (x > 20) {
                    return 84;
                    uint h = x + 42;
                }
            }
            if (x > 30) {
                return 42;
                uint b = 0xcafe;
            }
        }
        return 42;
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256): 0 -> 42
// f(uint256): 1 -> 42
// f(uint256): 2 -> 84
// f(uint256): 3 -> 84
// f(uint256): 4 -> 84
// f(uint256): 5 -> 84
// f(uint256): 6 -> 84
// f(uint256): 7 -> 84
// f(uint256): 8 -> 84
// f(uint256): 9 -> 84
// f(uint256): 10 -> 84
// f(uint256): 11 -> 84
