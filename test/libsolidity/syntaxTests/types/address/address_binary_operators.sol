contract C {
    address a;
    function f() public pure returns(bool) {
        a = address(0) + address(0);
        a = address(0) - address(0);
        a = address(0) * address(0);
        a = address(0) / address(0);
        return address(0) == address(0);
    }
}
// ----
// TypeError 2271: (85-108): Binary operator + not compatible with types address and address. Arithmetic operations on addresses are not supported. Convert to integer first before using them.
// TypeError 2271: (122-145): Binary operator - not compatible with types address and address. Arithmetic operations on addresses are not supported. Convert to integer first before using them.
// TypeError 2271: (159-182): Binary operator * not compatible with types address and address. Arithmetic operations on addresses are not supported. Convert to integer first before using them.
// TypeError 2271: (196-219): Binary operator / not compatible with types address and address. Arithmetic operations on addresses are not supported. Convert to integer first before using them.
