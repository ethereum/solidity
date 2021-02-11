contract test {
    function f() public {
        fixed16x2 a = 0; a;
        ufixed32x1 b = 0; b;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
