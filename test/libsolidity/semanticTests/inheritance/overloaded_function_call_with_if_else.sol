contract test {
    function f(uint256 a, uint256 b) public returns (uint256 d) {
        return a + b;
    }

    function f(uint256 k) public returns (uint256 d) {
        return k;
    }

    function g(bool flag) public returns (uint256 d) {
        if (flag) return f(3);
        else return f(3, 7);
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// g(bool): true -> 3
// g(bool): false -> 10
