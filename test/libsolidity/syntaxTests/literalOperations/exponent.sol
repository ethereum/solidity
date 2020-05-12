contract C {
    function g() public pure {
        int a;
        a ** 1E1233;
        a ** (1/2);
    }
}
// ----
// TypeError: (67-78): Operator ** not compatible with types int256 and int_const 1000...(1226 digits omitted)...0000. Exponent too large.
// TypeError: (88-98): Operator ** not compatible with types int256 and rational_const 1 / 2. Exponent is fractional.
