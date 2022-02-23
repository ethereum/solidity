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
// Warning 6838: (114-116): BMC: Condition is always true.
