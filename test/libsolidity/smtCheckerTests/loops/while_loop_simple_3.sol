pragma experimental SMTChecker;
// Check that condition is not assumed after the body anymore
contract C {
    function f(uint x) public pure {
        while (x == 2) {
        }
        assert(x == 2);
    }
}
// ----
// Warning: (187-201): Assertion violation happens here
