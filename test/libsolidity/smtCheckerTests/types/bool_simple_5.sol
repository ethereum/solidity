pragma experimental SMTChecker;
contract C {
    function f(bool x) public pure {
        bool y = x;
        assert(x == y);
    }
}
