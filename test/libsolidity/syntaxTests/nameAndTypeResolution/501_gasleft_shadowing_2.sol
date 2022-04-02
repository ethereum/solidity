contract C {
    uint gasleft;
    function f() public { gasleft = 42; }
}
// ----
// Warning 2319: (17-29='uint gasleft'): This declaration shadows a builtin symbol.
