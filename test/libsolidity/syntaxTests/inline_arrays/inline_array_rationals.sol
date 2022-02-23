contract test {
    function f() public {
        ufixed128x3[4] memory a = [ufixed128x3(3.5), 4.125, 2.5, 4.0];
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
// Warning 2072: (50-73): Unused local variable.
// Warning 2018: (20-118): Function state mutability can be restricted to pure
