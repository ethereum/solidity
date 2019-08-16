contract test {
    function f() public {
        fixed a = 4.5;
        ufixed d = 2.5;
        a; d;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
// Warning: (20-108): Function state mutability can be restricted to pure
