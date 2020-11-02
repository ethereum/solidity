pragma experimental SMTChecker;


contract C {
    int public x;
    int public y;

    function f() public pure {
        assert(this.x.selector != this.y.selector);
        assert(this.x.selector == this.y.selector);
    }
}
// ----
// Warning 6328: (175-217): CHC: Assertion violation happens here.
