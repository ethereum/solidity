contract test {
    function f(uint256 k) public returns (uint256) {
        return k;
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(uint256): 9 -> 9
