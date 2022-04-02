contract C {
    function f() public returns (uint a, uint b) {
        (a, b) += (1, 1);
    }
}
// ----
// TypeError 4289: (72-88='(a, b) += (1, 1)'): Compound assignment is not allowed for tuple types.
