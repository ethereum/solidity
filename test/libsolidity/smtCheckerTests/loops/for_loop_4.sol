pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        for (uint y = 2; x < 10; y = 3) {
            assert(y == 2);
        }
    }
}
// ====
// SMTSolvers: z3
// ----
// Warning 6328: (136-150): Assertion violation happens here
