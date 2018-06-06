contract test {
    function f() public returns (bool) { return g(12, true) == 3; }
    function g(uint256, bool) public returns (uint256) { }
}
// ----
// Warning: (88-142): Function state mutability can be restricted to pure
