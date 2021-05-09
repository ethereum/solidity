contract C
{
    function f() public view {
        assert(c > 0);
    }
    uint c;
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (52-65): CHC: Assertion violation happens here.\nCounterexample:\nc = 0\n\nTransaction trace:\nC.constructor()\nState: c = 0\nC.f()
