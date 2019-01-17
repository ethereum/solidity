pragma experimental SMTChecker;
// Check that variables are cleared
contract C {
    function f(uint x) public pure {
        x = 2;
        while (x > 1) {
            x = 1;
        }
        assert(x == 2);
    }
}
// ----
// Warning: (194-208): Assertion violation happens here
