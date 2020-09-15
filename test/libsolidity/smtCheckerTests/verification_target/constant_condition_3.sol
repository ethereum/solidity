pragma experimental SMTChecker;
// a plain literal constant is fine
contract C {
    function f(uint) public pure {
        if (true) { revert(); }
    }
}
// ----
