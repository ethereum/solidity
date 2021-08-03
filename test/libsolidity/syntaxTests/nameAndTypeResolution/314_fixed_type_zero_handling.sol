contract test {
    function f() public pure {
        fixed16x2 a = 0; a;
        ufixed32x1 b = 0; b;
    }
}
// ----
