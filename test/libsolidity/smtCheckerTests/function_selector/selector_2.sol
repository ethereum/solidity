contract C {
    function g() external pure {
    }

    function f() public pure {
        assert(msg.sig == this.g.selector);
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (92-126): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
