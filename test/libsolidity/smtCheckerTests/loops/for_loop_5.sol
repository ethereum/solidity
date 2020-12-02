pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        uint y;
        for (y = 2; x < 10; ) {
            y = 3;
        }
        assert(y == 3);
    }
}
// ====
// SMTSolvers: z3
// ----
// Warning 6328: (167-181): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 10\n\n\nTransaction trace:\nconstructor()\nf(10)
