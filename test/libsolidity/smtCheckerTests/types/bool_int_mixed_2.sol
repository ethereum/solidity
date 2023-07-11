contract C {
    function f(bool x, uint a) public pure {
        require(!x || a > 0);
        uint b = a;
        assert(!x || b > 0);
    }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
