contract C {
    // tests that bitwise operators are parsed from z3 answer
    function test(uint x, uint y) public pure {
        x | y;
        x & y;
        x ^ y;
        assert(true);
    }
}
// ====
// SMTEngine: all
// ----
