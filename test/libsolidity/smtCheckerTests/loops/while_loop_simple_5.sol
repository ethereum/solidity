pragma experimental SMTChecker;
// Check that side-effects of condition are taken into account
contract C {
    function f(uint x, uint y) public pure {
        x = 7;
        while ((x = y) > 0) {
        }
        assert(x == 7);
    }
}
// ----
// Warning: (216-230): Assertion violation happens here
