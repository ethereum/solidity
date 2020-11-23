contract C {
    function f(int256 a, uint256 b) public returns (int256) {
        return a << b;
    }

    function g(int256 a, uint256 b) public returns (int256) {
        return a >> b;
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(int256,uint256): 1, -1 -> 0
// g(int256,uint256): 1, -1 -> 0
