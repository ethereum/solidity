contract C {
    function f(uint x) public pure {
        if (x >= 10) { if (x < 10) { revert(); } }
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6838: (77-83): BMC: Condition is always false.
