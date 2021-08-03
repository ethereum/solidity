contract test {
    function f() public pure {
        fixed c = 3;
        ufixed d = 4;
        c; d;
    }
}
// ----
