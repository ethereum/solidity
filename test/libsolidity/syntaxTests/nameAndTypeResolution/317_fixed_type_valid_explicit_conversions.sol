contract test {
    function f() public {
        ufixed256x77 a = ufixed256x77(1/3); a;
        ufixed248x74 b = ufixed248x74(1/3); b;
        ufixed8x1 c = ufixed8x1(1/3); c;
    }
}
// ----
// Warning: (20-182): Function state mutability can be restricted to pure
