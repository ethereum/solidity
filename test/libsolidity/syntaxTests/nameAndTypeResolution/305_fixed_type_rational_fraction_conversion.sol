contract test {
    function f() public pure {
        fixed a = 4.5;
        ufixed d = 2.5;
        a; d;
    }
}
// ----
