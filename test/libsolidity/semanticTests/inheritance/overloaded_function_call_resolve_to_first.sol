contract test {
    function f(uint256 k) public returns (uint256 d) {
        return k;
    }

    function f(uint256 a, uint256 b) public returns (uint256 d) {
        return a + b;
    }

    function g() public returns (uint256 d) {
        return f(3);
    }
}
// ----
// g() -> 3
