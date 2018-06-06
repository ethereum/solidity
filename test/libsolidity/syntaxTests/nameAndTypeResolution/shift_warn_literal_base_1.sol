contract test {
    function f() pure public returns(uint) {
        uint8 x = 100;
        return 10 << x;
    }
}
// ----
// Warning: (99-106): Result of shift has type uint8 and thus might overflow. Silence this warning by converting the literal to the expected type.
