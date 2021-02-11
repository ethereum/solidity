contract A {
    fixed40x40 storeMe;
    function f(ufixed x, fixed32x32 y) public {
        ufixed8x8 a;
        fixed b;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
