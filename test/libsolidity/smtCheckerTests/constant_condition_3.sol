pragma experimental SMTChecker;
// a plain literal constant is fine
contract C {
    function f(uint) public pure {
        if (true) { revert(); }
    }
}
// ----
// Warning: (136-144): Assertion checker does not yet implement this type of function call.
