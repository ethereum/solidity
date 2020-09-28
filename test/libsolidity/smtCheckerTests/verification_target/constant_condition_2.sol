pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        if (x >= 10) { if (x < 10) { revert(); } }
    }
}
// ----
// Warning 6838: (109-115): BMC: Condition is always false.
