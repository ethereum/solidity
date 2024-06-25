contract C {
    uint transient x = 42;
    function test() public view { assert(x == 42); }
}
// ====
// SMTEngine: all
// ----
// UnimplementedFeatureError 6715: (17-38): Transient storage is not yet implemented.
