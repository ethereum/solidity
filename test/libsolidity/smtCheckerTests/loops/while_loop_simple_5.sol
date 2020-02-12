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
// Warning: (224-238): Assertion violation happens here
