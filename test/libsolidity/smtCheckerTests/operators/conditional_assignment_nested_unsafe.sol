pragma experimental SMTChecker;

contract C {
    function f(bool b1, bool b2) public pure {
        uint c = b1 ? 3 : (b2 ? 2 : 1);
        assert(c > 1);
    }
}
// ----
// Warning 6328: (141-154): Assertion violation happens here
