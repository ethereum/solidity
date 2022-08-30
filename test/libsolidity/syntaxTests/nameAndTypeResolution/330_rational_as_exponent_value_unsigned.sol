contract test {
    function f() public {
        ufixed b = 3 ** 2.5;
    }
}
// ----
// TypeError 2271: (61-69): Binary operator ** not compatible with types int_const 3 and rational_const 5 / 2.
