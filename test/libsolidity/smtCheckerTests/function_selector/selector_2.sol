pragma experimental SMTChecker;

contract C {
    function g() external pure {
    }

    function f() public pure {
        assert(msg.sig == this.g.selector);
    }
}
// ----
// Warning 6328: (125-159): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
