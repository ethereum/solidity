contract C {
    function f(uint x, uint y) public pure {
        assembly {}
        assert(x < y);
    }

    function g(uint x, uint y) public pure {
        assembly {}
        assert(x < y);
    }
}
// ====
// SMTEngine: all
// SMTShowUnsupported: no
// ----
// Warning 5724: SMTChecker: 2 unsupported language feature(s). Enable the model checker option "show unsupported" to see all of them.
// Warning 6328: (86-99): CHC: Assertion violation happens here.
// Warning 6328: (181-194): CHC: Assertion violation happens here.
