contract C {
    function f(bool x, bool y) public pure {
        assert(x == y);
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (66-80): CHC: Assertion violation happens here.\nCounterexample:\n\nx = true\ny = false\n\nTransaction trace:\nC.constructor()\nC.f(true, false)
