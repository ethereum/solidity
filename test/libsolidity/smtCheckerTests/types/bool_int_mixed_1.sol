contract C {
    function f(bool x) public pure {
        uint a;
        if(x)
            a = 1;
        assert(!x || a > 0);
    }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
