contract C {
    event E();
    function f() public pure {
        revert E();
    }
}
// ----
// TypeError 1885: (74-75='E'): Expression has to be an error.
