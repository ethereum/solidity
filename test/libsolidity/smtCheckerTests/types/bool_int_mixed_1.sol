pragma experimental SMTChecker;
contract C {
    function f(bool x) public pure {
        uint a;
        if(x)
            a = 1;
        assert(!x || a > 0);
    }
}
