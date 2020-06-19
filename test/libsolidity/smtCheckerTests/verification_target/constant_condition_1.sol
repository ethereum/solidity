pragma experimental SMTChecker;
contract C {
    function f(uint x) public pure {
        if (x >= 0) { revert(); }
    }
}
// ----
// Warning 6838: (94-100): Condition is always true.
// Warning 4588: (104-112): Assertion checker does not yet implement this type of function call.
