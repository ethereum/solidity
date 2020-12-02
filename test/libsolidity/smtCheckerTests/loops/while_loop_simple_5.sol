pragma experimental SMTChecker;
// Check that side-effects of condition are taken into account
contract C {
    function f(uint x, uint y) public pure {
        x = 7;
        while ((x = y) > 0) {
			--y;
        }
        assert(x == 7);
    }
}
// ====
// SMTSolvers: z3
// ----
// Warning 6328: (224-238): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\ny = 0\n\n\nTransaction trace:\nconstructor()\nf(0, 0)
