pragma experimental SMTChecker;
contract C {
    function f(bool x, uint a) public pure {
        require(!x || a > 0);
        uint b = a;
        assert(!x || b > 0);
    }
}
