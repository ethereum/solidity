contract C {
    function f(uint a) public pure { assert(a == 2); }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (50-64): CHC: Assertion violation happens here.\nCounterexample:\n\na = 0\n\nTransaction trace:\nC.constructor()\nC.f(0)
