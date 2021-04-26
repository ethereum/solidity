contract C {
    function f(uint x) public pure {
        require(x == 2);
        for (; x > 2;) {}
        assert(x == 2);
    }
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 6838: (90-95): BMC: Condition is always false.
