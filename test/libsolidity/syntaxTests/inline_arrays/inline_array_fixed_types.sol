contract test {
    function f() public {
        fixed[3] memory a = [fixed(3.5), fixed(-4.25), fixed(967.125)];
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
// Warning 2072: (50-67): Unused local variable.
// Warning 2018: (20-119): Function state mutability can be restricted to pure
