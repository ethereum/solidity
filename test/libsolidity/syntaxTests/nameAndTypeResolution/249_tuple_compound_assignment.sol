contract C {
    function f() public returns (uint a, uint b) {
        (a, b) += (1, 1);
    }
}
// ----
// TypeError: (72-88): Compound assignment is not allowed for tuple types.
