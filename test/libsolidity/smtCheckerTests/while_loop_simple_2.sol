pragma experimental SMTChecker;
// Check that condition is assumed.
contract C {
    function f(uint x) public pure {
        while (x == 2) {
            assert(x == 2);
        }
    }
}
