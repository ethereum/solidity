contract test {
    function f() public pure {
        ufixed256x18 a = ufixed256x18(1/3); a;
        ufixed248x18 b = ufixed248x18(1/3); b;
        ufixed8x1 c = ufixed8x1(1/3); c;
    }
}
// ----
