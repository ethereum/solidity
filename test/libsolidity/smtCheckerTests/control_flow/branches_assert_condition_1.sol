contract C {
    function f(uint x) public pure {
        if (x > 10) {
            assert(x > 9);
        }
        else
        {
            assert(x < 11);
        }
    }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
