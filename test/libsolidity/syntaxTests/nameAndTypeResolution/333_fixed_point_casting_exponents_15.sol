contract test {
    function f() public {
        ufixed a = 3 ** ufixed(1.5);
    }
}
// ----
// TypeError 2271: (61-77): Operator ** not compatible with types int_const 3 and ufixed128x18. Exponent is fractional.
