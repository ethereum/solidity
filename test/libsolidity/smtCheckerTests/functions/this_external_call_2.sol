pragma experimental SMTChecker;

contract C {
    uint a;
    function f(uint x) public {
        this.g(x);
        assert(a == x);
        assert(a != 42);
    }

    function g(uint x) public {
        a = x;
    }
}
// ----
// Warning 6328: (141-156): CHC: Assertion violation happens here.\nCounterexample:\na = 42\nx = 42\n\nTransaction trace:\nC.constructor()\nState: a = 0\nC.f(42)\n    C.g(42) -- trusted external call
