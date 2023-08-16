contract C {

    function f() public pure {
        uint x = 1;
        assert(x == 2);
    }
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (73-87): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 1\n\nTransaction trace:\nC.constructor()\nC.f()
