pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        uint a = 3;
        if (x > 10) {
            ++a;
        }
        assert(a == 3);
    }
}
// ----
// Warning 6328: (159-173): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 11\na = 4\n\nTransaction trace:\nC.constructor()\nC.f(11)
