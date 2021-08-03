contract test {
    function f() public pure {
        ufixed256x77 a = ufixed256x77(1/3); a;
        ufixed248x77 b = ufixed248x77(1/3); b;
        ufixed8x1 c = ufixed8x1(1/3); c;
    }
}
// ----
