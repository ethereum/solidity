contract test {
    function f() public pure {
        ufixed64x8 a = 3.5 * 3;
        ufixed64x8 b = 4 - 2.5;
        ufixed64x8 c = 11 / 4;
        ufixed240x5 d = 599 + 0.21875;
        ufixed256x18 e = ufixed256x18(35.245 % 12.9);
        ufixed256x18 f1 = ufixed256x18(1.2 % 2);
        fixed g = 2 ** -2;
        a; b; c; d; e; f1; g;
    }
}
// ----
