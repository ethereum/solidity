contract C {
    function f(uint x) public pure {
        for (; x == 2; ) {
            assert(x == 2);
        }
    }
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
