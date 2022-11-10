contract test {
    function f() public {
        42 ** (-1/4);
    }
}
// ----
// TypeError 2271: (50-62): Built-in binary operator ** cannot be applied to types int_const 42 and rational_const -1 / 4.
