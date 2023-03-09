contract C {
    function f(bool b1, bool b2) public pure {
        require(b1 || b2);
        uint c = b1 ? 3 : (b2 ? 2 : 1);
        assert(c > 1);
    }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
// Warning 6838: (114-116): BMC: Condition is always true.
