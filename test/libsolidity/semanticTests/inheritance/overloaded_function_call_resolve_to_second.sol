contract test {
    function f(uint256 a, uint256 b) public returns (uint256 d) {
        return a + b;
    }

    function f(uint256 k) public returns (uint256 d) {
        return k;
    }

    function g() public returns (uint256 d) {
        return f(3, 7);
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// g() -> 10
