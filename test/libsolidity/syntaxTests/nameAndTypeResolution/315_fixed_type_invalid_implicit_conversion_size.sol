contract test {
    function f() public {
        ufixed a = 11/4;
        ufixed248x8 b = a; b;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
