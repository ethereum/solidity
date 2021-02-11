contract test {
    function f() public {
        ufixed b = 3 ** 2.5;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
