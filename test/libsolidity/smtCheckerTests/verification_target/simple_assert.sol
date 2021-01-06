pragma experimental SMTChecker;
contract C {
    function f(uint a) public pure { assert(a == 2); }
}
// ----
// Warning 6328: (82-96): CHC: Assertion violation happens here.\nCounterexample:\n\na = 0\n\nTransaction trace:\nC.constructor()\nC.f(0)
