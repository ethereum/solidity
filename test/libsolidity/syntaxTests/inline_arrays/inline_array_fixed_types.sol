contract test {
    function f() public pure {
        fixed[3] memory a = [fixed(3.5), fixed(-4.25), fixed(967.125)];
        a;
    }
}
// ----
