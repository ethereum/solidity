pragma experimental SMTChecker;
contract C {
    uint[1] c;
    function f(bool b) public {
        require(c[0] == 0);
        if (b)
            c[0] |= 1;
        assert(c[0] <= 1);
    }
}
// ----
// Warning 6328: (166-183): CHC: Assertion violation might happen here.
