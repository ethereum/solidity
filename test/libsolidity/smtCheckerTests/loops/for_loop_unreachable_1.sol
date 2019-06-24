pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        require(x == 2);
        for (; x > 2;) {}
        assert(x == 2);
    }
}
// ----
// Warning: (122-127): Condition is always false.
