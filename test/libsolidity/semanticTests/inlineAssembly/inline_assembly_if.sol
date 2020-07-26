contract C {
    function f(uint256 a) public returns (uint256 b) {
        assembly {
            if gt(a, 1) {
                b := 2
            }
        }
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f(uint256): 0 -> 0
// f(uint256): 1 -> 0
// f(uint256): 2 -> 2
// f(uint256): 3 -> 2
