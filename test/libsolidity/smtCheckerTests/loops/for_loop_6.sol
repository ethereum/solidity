pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        uint y;
        for (y = 2; x < 10; ) {
            y = 3;
        }
        // False positive due to resetting y.
        assert(y < 4);
    }
}
// ----
