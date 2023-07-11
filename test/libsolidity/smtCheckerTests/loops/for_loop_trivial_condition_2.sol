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
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
