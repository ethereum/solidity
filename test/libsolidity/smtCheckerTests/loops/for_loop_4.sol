pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        for (uint y = 2; x < 10; y = 3) {
            assert(y == 2);
        }
    }
}
// ====
// SMTSolvers: z3
// ----
// Warning 6328: (136-150): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\n\n\nTransaction trace:\nconstructor()\nf(0)
