contract C {
    uint transient x = 42;
    function test() public view { assert(x == 42); }
}
// ====
// SMTEngine: all
// ----
// UnimplementedFeatureError 1834: Transient storage variables are not supported.
