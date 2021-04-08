contract C {
    function f(uint x) public pure {
        if (x >= 0) { revert(); }
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6838: (62-68): BMC: Condition is always true.
