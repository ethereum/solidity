contract test {
    function f() public {
        ufixed256x77 a = ufixed256x77(1/3); a;
        ufixed248x77 b = ufixed248x77(1/3); b;
        ufixed8x1 c = ufixed8x1(1/3); c;
    }
}
// ----
// UnimplementedFeatureError: Not yet implemented - FixedPointType.
// Warning 2018: (20-182): Function state mutability can be restricted to pure
