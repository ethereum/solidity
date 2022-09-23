contract test {
    function f() public {
        ufixed b = 3 ** 2.5;
    }
}
// ----
// TypeError 2271: (61-69): Built-in binary operator ** cannot be applied to types int_const 3 and rational_const 5 / 2.
