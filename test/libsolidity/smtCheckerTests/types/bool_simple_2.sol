pragma experimental SMTChecker;
contract C {
    function f(bool x, bool y) public pure {
        assert(x == y);
    }
}
// ----
// Warning 4661: (98-112): Assertion violation happens here
