pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        require(x == 2);
        for (;;) {}
        assert(x == 2);
    }
}
