pragma experimental SMTChecker;
// Check that condition is not assumed after the body anymore
contract C {
    function f(uint x) public pure {
        while (x == 2) {
        }
        assert(x == 2);
    }
}
// ====
// SMTSolvers: z3
// ----
// Warning 6328: (187-201): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 3\n\n\nTransaction trace:\nconstructor()\nf(3)
