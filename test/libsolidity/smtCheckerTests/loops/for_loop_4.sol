contract C {
    function f(uint x) public pure {
        for (uint y = 2; x < 10; y = 3) {
            assert(y == 2);
        }
    }
}
// ====
// SMTEngine: all
// SMTSolvers: z3
// ----
// Warning 6328: (104-118): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0\ny = 3\n\nTransaction trace:\nC.constructor()\nC.f(0)
