contract test {
    function f() public {
        fixed(1.75) & 3;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
