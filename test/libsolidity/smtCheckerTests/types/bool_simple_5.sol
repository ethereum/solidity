contract C {
    function f(bool x) public pure {
        bool y = x;
        assert(x == y);
    }
}
// ====
// SMTEngine: all
// ----
