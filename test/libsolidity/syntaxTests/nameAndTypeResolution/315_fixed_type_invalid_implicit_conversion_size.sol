contract test {
    function f() public {
        ufixed a = 11/4;
        ufixed248x8 b = a; b;
    }
}
// ----
// TypeError: (75-92): Type ufixed128x18 is not implicitly convertible to expected type ufixed248x8. Too many fractional digits.
