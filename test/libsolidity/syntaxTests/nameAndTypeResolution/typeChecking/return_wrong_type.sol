contract test {
    function f() public returns (uint256 r) { return 1 >= 2; }
}
// ----
// TypeError 6359: (69-75): Return argument type bool is not implicitly convertible to expected type uint256.
