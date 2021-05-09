error revert();
contract C {
    function f() public pure {
        revert revert();
    }
}
// ----
// Warning 2319: (0-15): This declaration shadows a builtin symbol.
