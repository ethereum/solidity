pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        for (; x == 2; ) {
            assert(x == 2);
        }
    }
}
