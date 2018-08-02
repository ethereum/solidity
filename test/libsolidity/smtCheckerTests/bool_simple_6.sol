pragma experimental SMTChecker;
contract C {
    function f(bool x) public pure {
        require(x);
        bool y;
        y = false;
        assert(x || y);
    }
}
