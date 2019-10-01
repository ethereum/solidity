pragma experimental SMTChecker;
// Check that negation of condition is not assumed after the body anymore
contract C {
    function f(uint x) public pure {
        while (x == 2) {
        }
        assert(x != 2);
    }
}
// ----
