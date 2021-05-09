contract C {
    function f() public pure {
        bytes3 y = "def";
        y &= "def";
        assert(y == "def");

        y |= "dee";
        assert(y == "def"); // fails

        y ^= "fed";
        assert(y == (bytes3("def") | bytes3("dee")) ^ bytes3("fed"));
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (147-165): CHC: Assertion violation happens here.\nCounterexample:\n\ny = 6579559\n\nTransaction trace:\nC.constructor()\nC.f()
