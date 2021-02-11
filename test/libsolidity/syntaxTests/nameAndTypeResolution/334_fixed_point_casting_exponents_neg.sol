contract test {
    function f() public {
        ufixed c = 42 ** fixed(-1/4);
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
