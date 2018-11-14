pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        if (x > 10) {
            assert(x > 9);
        }
        else if (x > 2)
        {
            assert(x <= 10 && x > 2);
        }
        else
        {
           assert(0 <= x && x <= 2);
        }
    }
}
