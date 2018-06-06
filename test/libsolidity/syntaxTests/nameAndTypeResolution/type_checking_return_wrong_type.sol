contract test {
    function f() public returns (uint256 r) { return 1 >= 2; }
}
// ----
// TypeError: (69-75): Return argument type bool is not implicitly convertible to expected type (type of first return variable) uint256.
