pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        uint y;
        for (y = 2; x < 10; ) {
            y = 3;
        }
        assert(y == 3);
    }
}
// ----
// Warning: (167-181): Assertion violation happens here\nNote that some information is erased after the execution of loops.\nYou can re-introduce information using require().
