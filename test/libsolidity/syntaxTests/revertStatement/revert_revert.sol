error revert();
contract C {
    function f() public pure {
        revert revert();
    }
}
// ----
// Warning 2319: (0-15='error revert();'): This declaration shadows a builtin symbol.
