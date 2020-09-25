pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        if (x >= 0) { revert(); }
    }
}
// ----
// Warning 6838: (94-100): BMC: Condition is always true.
