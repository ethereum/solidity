pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        uint a = 3;
        if (x > 10) {
            a = 5;
        }
        assert(a == 3);
    }
}
// ----
// Warning: (161-175): Assertion violation happens here
