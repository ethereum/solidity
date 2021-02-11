contract test {
    function f() public {
        fixed g = 2 ** -2.2;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
