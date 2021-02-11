contract test {
    function f() public {
        ufixed a = 3 ** ufixed(1.5);
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
