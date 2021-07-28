contract test {
    function f() public {
        ufixed a = 11/4;
        ufixed248x8 b = a; b;
    }
}
// ----
// TypeError 9574: (75-92): Type ufixed128x18 is not implicitly convertible to expected type ufixed248x8. Conversion would incur precision loss - use explicit conversion instead.
