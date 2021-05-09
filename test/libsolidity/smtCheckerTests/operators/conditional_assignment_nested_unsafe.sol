contract C {
    function f(bool b1, bool b2) public pure {
        uint c = b1 ? 3 : (b2 ? 2 : 1);
        assert(c > 1);
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (108-121): CHC: Assertion violation happens here.\nCounterexample:\n\nb1 = false\nb2 = false\nc = 1\n\nTransaction trace:\nC.constructor()\nC.f(false, false)
