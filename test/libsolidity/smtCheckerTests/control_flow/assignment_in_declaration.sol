contract C {
    function f() public pure { uint a = 2; assert(a == 2); }
}
// ====
// SMTEngine: all
// ----
