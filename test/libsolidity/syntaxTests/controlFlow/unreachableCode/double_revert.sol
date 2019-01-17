contract C {
    function f() public pure {
        revert();
        revert();
    }
}
// ----
// Warning: (70-78): Unreachable code.
