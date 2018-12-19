contract c {
    function f() public pure {
        int a;
        a = 1/(2<<4094)/(2<<4094);
    }
}
// ----
// TypeError: (71-92): Operator / not compatible with types rational_const 1 / 5221...(1225 digits omitted)...5168 and int_const 5221...(1225 digits omitted)...5168. Precision of rational constants is limited to 4096 bits.
// TypeError: (71-92): Type rational_const 1 / 5221...(1225 digits omitted)...5168 is not implicitly convertible to expected type int256. Try converting to type ufixed8x80 or use an explicit conversion.
