contract test {
    function f() public {
        fixed g = 2 ** -2.2;
    }
}
// ----
// TypeError 2271: (60-69): Built-in binary operator ** cannot be applied to types int_const 2 and rational_const -11 / 5.
