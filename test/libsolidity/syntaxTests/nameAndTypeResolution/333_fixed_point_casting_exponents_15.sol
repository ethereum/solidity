contract test {
    function f() public {
        var a = 3 ** ufixed(1.5);
    }
}
// ----
// Warning: (50-55): Use of the "var" keyword is deprecated.
// TypeError: (58-74): Operator ** not compatible with types int_const 3 and ufixed128x18
// Warning: (50-74): The type of this variable was inferred as uint8, which can hold values between 0 and 255. This is probably not desired. Use an explicit type to silence this warning.
