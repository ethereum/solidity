contract test {
    function f() public {
        ufixed256x80 a = ufixed256x80(1/3); a;
        ufixed248x80 b = ufixed248x80(1/3); b;
        ufixed8x1 c = ufixed8x1(1/3); c;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
// Warning: (20-182): Function state mutability can be restricted to pure
