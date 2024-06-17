contract test {
    function f() public {
        fixed16x2 a = 0; a;
        ufixed32x1 b = 0; b;
    }
}
// ----
// Warning 2018: (20-104): Function state mutability can be restricted to pure
// UnimplementedFeatureError 1834: Not yet implemented - FixedPointType.
