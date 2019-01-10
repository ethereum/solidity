contract C {
    function f() public pure {
        revert();
        uint a = 0; a;
    }
}
// ----
// Warning: (70-83): Unreachable code.
