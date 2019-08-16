contract test {
    function f() public {
        ufixed a = uint64(1) + ufixed(2);
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
// Warning: (50-58): Unused local variable.
// Warning: (20-89): Function state mutability can be restricted to pure
