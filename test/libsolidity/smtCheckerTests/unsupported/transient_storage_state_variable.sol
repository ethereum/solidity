contract C {
    uint transient x;
    function test() public view { assert(x == 0); }
}
// ====
// SMTEngine: all
// ----
// UnimplementedFeatureError 1834: Transient storage variables are not supported.
