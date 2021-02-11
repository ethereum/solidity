contract test {
    function f() public {
        ufixed a = uint64(1) + ufixed(2);
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
