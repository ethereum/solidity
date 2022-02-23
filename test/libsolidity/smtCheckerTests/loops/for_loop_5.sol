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
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 6328: (135-149): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 10\ny = 2\n\nTransaction trace:\nC.constructor()\nC.f(10)
