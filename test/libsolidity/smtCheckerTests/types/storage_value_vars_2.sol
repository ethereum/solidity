contract C
{
    address a;
    bool b;
    uint c;
    function f() public view {
        assert(c > 0);
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (91-104): CHC: Assertion violation happens here.\nCounterexample:\na = 0, b = false, c = 0\n\nTransaction trace:\nC.constructor()\nState: a = 0, b = false, c = 0\nC.f()
