contract A {
    fixed40x40 storeMe;
    function f(ufixed x, fixed32x32 y) public {
        ufixed8x8 a;
        fixed b;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
// Warning: (52-60): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (62-74): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (93-104): Unused local variable.
// Warning: (114-121): Unused local variable.
// Warning: (41-128): Function state mutability can be restricted to pure
