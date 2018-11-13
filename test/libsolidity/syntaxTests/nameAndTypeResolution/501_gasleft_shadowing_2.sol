contract C {
    uint gasleft;
    function f() public { gasleft = 42; }
}
// ----
// Warning: (17-29): This declaration shadows a builtin symbol.
