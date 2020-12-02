pragma experimental SMTChecker;
// Check that variables are cleared
contract C {
    function f(uint x) public pure {
        x = 2;
        while (x > 1) {
            x = 1;
        }
        assert(x == 2);
    }
}
// ====
// SMTSolvers: z3
// ----
// Warning 6328: (194-208): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 1\n\n\nTransaction trace:\nconstructor()\nf(0)
