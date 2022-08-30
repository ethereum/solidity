contract C {
    function g() public pure {
        int a;
        a ** 1E1233;
        a ** (1/2);
    }
}
// ----
// TypeError 2271: (67-78): Binary operator ** not compatible with types int256 and int_const 1000...(1226 digits omitted)...0000. Exponent too large.
// TypeError 2271: (88-98): Binary operator ** not compatible with types int256 and rational_const 1 / 2. Exponent is fractional.
