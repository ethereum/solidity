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
// ====
// SMTSolvers: z3
// ----
// Warning 6838: (138-144): Condition is always true.
