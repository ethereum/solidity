pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        require(x == 2);
        uint y;
        // The loop condition is always true,
        // but since x is touched in the body
        // we can't infer that.
        for (; x == 2;) {
            x = 2;
        }
        // False positive due to resetting x.
        assert(x == 2);
    }
}
// ----
// Warning: (115-121): Unused local variable.
// Warning: (356-370): Assertion violation happens here
