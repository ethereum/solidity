contract test {
    function f() public returns (bool) { return g(12, true) == 3; }
    function g(uint256, bool) public returns (uint256) { }
}
// ----
