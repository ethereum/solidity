pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        if (x > 10) {
            assert(x > 9);
        }
        else
        {
            assert(x < 11);
        }
    }
}
