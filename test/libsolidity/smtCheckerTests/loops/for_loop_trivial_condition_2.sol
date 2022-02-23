contract C {
    function f(uint x) public pure {
        require(x == 2);
        uint y;
        for (; x == 2;) {
            y = 7;
        }
        assert(x == 2);
    }
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 6838: (106-112): BMC: Condition is always true.
