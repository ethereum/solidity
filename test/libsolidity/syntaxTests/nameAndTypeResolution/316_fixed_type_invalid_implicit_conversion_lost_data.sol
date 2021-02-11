contract test {
    function f() public {
        ufixed256x1 a = 1/3; a;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
