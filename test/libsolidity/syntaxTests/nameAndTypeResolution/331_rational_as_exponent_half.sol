contract test {
    function f() public {
        2 ** (1/2);
    }
}
// ----
// TypeError 2271: (50-60): Binary operator ** not compatible with types int_const 2 and rational_const 1 / 2.
