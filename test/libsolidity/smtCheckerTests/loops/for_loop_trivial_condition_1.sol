pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        require(x == 2);
        for (; x == 2;) {}
        assert(x == 2);
    }
}
// ====
// SMTSolvers: z3
// ----
// Warning 6838: (122-128): Condition is always true.
