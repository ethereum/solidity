pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        if (x >= 10) { if (x < 10) { revert(); } }
    }
}
// ----
// Warning: (109-115): Condition is always false.
// Warning: (119-127): Assertion checker does not yet implement this type of function call.
