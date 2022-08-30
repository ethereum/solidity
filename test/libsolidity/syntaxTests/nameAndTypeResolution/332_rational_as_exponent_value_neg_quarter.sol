contract test {
    function f() public {
        42 ** (-1/4);
    }
}
// ----
// TypeError 2271: (50-62): Binary operator ** not compatible with types int_const 42 and rational_const -1 / 4.
