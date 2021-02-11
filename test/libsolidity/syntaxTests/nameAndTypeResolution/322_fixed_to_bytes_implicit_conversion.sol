contract test {
    function f() public {
        fixed a = 3.25;
        bytes32 c = a; c;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
