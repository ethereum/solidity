contract C {
    function gasleft() public pure returns (bytes32 val) { return "abc"; }
    function f() public pure returns (bytes32 val) { return gasleft(); }
}
// ----
// Warning: (17-87): This declaration shadows a builtin symbol.
