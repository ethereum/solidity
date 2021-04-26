contract C {
    function f(bool x) public pure {
        if(x) {
            assert(x);
        } else {
            assert(!x);
        }
    }
}
// ====
// SMTEngine: all
// ----
