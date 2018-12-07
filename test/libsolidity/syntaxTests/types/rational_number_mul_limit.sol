contract c {
    function f() public pure {
        int a;
        a = (1<<4095)*(1<<4095);
    }
}
// ----
// TypeError: (71-90): Operator * not compatible with types int_const 5221...(1225 digits omitted)...5168 and int_const 5221...(1225 digits omitted)...5168. Precision of rational constants is limited to 4096 bits.
// TypeError: (71-90): Type int_const 5221...(1225 digits omitted)...5168 is not implicitly convertible to expected type int256.
