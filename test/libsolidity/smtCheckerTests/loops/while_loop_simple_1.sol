pragma experimental SMTChecker;
// Check that variables are cleared
contract C {
    function f(uint x) public pure {
        x = 2;
        while (x > 1) {
            x = 2;
        }
        assert(x == 2);
    }
}
// ----
// Warning: (194-208): Assertion violation happens here\nNote that some information is erased after the execution of loops.\nYou can re-introduce information using require().
