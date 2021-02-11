contract test {
    function f() public {
        ufixed128x3[4] memory a = [ufixed128x3(3.5), 4.125, 2.5, 4.0];
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
