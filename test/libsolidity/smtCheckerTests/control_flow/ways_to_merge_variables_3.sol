pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        uint a = 3;
        if (x > 10) {
            a = 5;
        }
        assert(a == 3);
    }
}
// ----
// Warning 6328: (161-175): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 11\na = 5\n\nTransaction trace:\nC.constructor()\nC.f(11)
