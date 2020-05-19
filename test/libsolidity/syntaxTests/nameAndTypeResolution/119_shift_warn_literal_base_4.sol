contract test {
    function f() pure public returns(uint) {
         uint8 x = 100;
         return 10 >> x;
    }
}
// ----
// TypeError: (101-108): Operator >> not compatible with types int_const 10 and uint8. Shift operators need an explicit type for the base.
