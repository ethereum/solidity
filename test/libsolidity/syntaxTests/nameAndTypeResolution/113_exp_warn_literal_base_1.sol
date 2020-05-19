contract test {
    function f() pure public returns(uint) {
        uint8 x = 100;
        return 10**x;
    }
}
// ----
// TypeError: (99-104): Operator ** not compatible with types int_const 10 and uint8. Exponentiation needs an explicit type for the base.
