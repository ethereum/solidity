contract test {
    function f() public {
        fixed a = 4.5;
        ufixed d = 2.5;
        a; d;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
