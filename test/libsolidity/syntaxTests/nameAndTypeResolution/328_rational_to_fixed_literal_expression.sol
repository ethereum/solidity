contract test {
    function f() public {
        ufixed64x8 a = 3.5 * 3;
        ufixed64x8 b = 4 - 2.5;
        ufixed64x8 c = 11 / 4;
        ufixed240x5 d = 599 + 0.21875;
        ufixed256x80 e = ufixed256x80(35.245 % 12.9);
        ufixed256x80 f = ufixed256x80(1.2 % 2);
        fixed g = 2 ** -2;
        a; b; c; d; e; f; g;
    }
}
// ----
// Warning: (238-252): This declaration shadows an existing declaration.
// Warning: (20-339): Function state mutability can be restricted to pure
