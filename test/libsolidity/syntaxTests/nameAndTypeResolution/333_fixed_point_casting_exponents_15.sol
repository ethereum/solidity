contract test {
    function f() public {
        ufixed a = 3 ** ufixed(1.5);
    }
}
// ----
// TypeError 2271: (61-77): Built-in binary operator ** cannot be applied to types int_const 3 and ufixed128x18. Exponent is fractional.
