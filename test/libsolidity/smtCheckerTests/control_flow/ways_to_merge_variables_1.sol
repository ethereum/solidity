pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        uint a = 3;
        if (x > 10) {
            a++;
        }
        assert(a == 3);
    }
}
// ----
// Warning: (159-173): Assertion violation happens here
