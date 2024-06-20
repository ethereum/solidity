contract test {
    function f() public {
        ufixed256x80 a = ufixed256x80(1/3); a;
        ufixed248x80 b = ufixed248x80(1/3); b;
        ufixed8x1 c = ufixed8x1(1/3); c;
    }
}
// ----
// Warning 2018: (20-182): Function state mutability can be restricted to pure
// UnimplementedFeatureError 1834: Not yet implemented - FixedPointType.
