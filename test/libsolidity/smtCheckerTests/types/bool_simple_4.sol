contract C {
    function f(bool x) public pure {
        if(x) {
            assert(x);
        } else {
            assert(!x);
        }
    }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
