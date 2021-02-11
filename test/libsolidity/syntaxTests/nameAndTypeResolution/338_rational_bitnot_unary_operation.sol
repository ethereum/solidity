contract test {
    function f() public {
        ~fixed(3.5);
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
