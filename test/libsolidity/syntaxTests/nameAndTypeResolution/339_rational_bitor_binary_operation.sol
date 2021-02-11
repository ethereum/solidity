contract test {
    function f() public {
        fixed(1.5) | 3;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
