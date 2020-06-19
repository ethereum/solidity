contract C {
    function f() public pure {
        var (d, e,);
    }
}
// ----
// TypeError 4626: (52-63): Use of the "var" keyword is disallowed.
