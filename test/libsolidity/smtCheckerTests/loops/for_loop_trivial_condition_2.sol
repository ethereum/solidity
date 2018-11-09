pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        require(x == 2);
        uint y;
        for (; x == 2;) {
            y = 7;
        }
        assert(x == 2);
    }
}
// ----
// Warning: (138-144): For loop condition is always true.
