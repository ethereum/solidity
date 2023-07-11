contract C {
    function f(bool x, bool y) public pure {
        bool z = x || y;
        assert(!(x && y) || z);
    }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
