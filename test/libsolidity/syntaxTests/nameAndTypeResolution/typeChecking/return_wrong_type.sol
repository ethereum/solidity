contract test {
    function f() public returns (uint256 r) { return 1 >= 2; }
}
// ----
// TypeError 6359: (69-75='1 >= 2'): Return argument type bool is not implicitly convertible to expected type (type of first return variable) uint256.
