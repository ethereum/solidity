pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        for (uint y = 2; x < 10; ) {
            assert(y == 2);
        }
    }
}
